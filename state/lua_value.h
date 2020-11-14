#pragma once
#include "api/consts.h"
#include "number/parser.h"
#include <memory>
#include <unordered_map>

struct LuaTable;
using LuaTablePtr = std::shared_ptr<LuaTable>;

struct LuaValue
{
	LuaType tag;
	union
	{
		bool boolean;
		Int64 integer;
		Float64 number;
	};
	bool isfloat;
	std::string str;
	LuaTablePtr table;

	const static LuaValue NoValue;
	const static LuaValue Nil;

	inline bool IsInt64() const { return tag == LUA_TNUMBER && !isfloat; }
	inline bool IsFloat64() const { return tag == LUA_TNUMBER && isfloat; }
	inline bool IsBool() const { return tag == LUA_TBOOLEAN; }
	inline bool IsString() const { return tag == LUA_TSTRING; }
	inline bool IsTable() const { return tag == LUA_TTABLE; }

	static size_t _BKDR(const char* pData, size_t uLen)
	{
		size_t seed = 31; // 31 131 1313 13131 131313 etc.. 37
		size_t hash = 0; 
		/* variant with the hash unrolled eight times */ 
		for (; uLen >= 8; uLen -= 8)
		{ 
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
			hash = hash * seed + *pData++;
		} 
		switch (uLen)
		{
			case 7: hash = hash * seed + *pData++; /* fallthrough... */
			case 6: hash = hash * seed + *pData++; /* fallthrough... */
			case 5: hash = hash * seed + *pData++; /* fallthrough... */
			case 4: hash = hash * seed + *pData++; /* fallthrough... */
			case 3: hash = hash * seed + *pData++; /* fallthrough... */
			case 2: hash = hash * seed + *pData++; /* fallthrough... */
			case 1: hash = hash * seed + *pData++; break; 
			case 0: break;
		}
	
		return hash;
	}

	size_t Hash() const
	{
		size_t hash = 0;
		HashCombine(hash, tag);
		HashCombine(hash, integer);
		HashCombine(hash, isfloat);
		HashCombine(hash, _BKDR(str.c_str(), str.length()));
		HashCombine(hash, table ? (size_t)table.get() : 0);
		return hash;
	}

	bool operator==(const LuaValue& rhs) const { return Hash() == rhs.Hash(); }
	bool operator<(const LuaValue& rhs) const { return Hash() < rhs.Hash(); }
	bool operator<=(const LuaValue& rhs) const { return Hash() <= rhs.Hash(); }
	bool operator!=(const LuaValue& rhs) const { return !(*this == rhs); }
	bool operator>(const LuaValue& rhs) const { return !(*this <= rhs); }
	bool operator>=(const LuaValue& rhs) const { return !(*this < rhs); }

	LuaValue()
	{
		tag = LUA_TNONE;
		integer = false;
		isfloat = false;
	}

	LuaValue(LuaType _tag)
	{
		tag = _tag;
		integer = false;
		isfloat = false;
	}

	LuaValue(bool value)
	{
		tag = LUA_TBOOLEAN;
		boolean = value;
		isfloat = false;
	}

	LuaValue(Int64 value)
	{
		tag = LUA_TNUMBER;
		integer = value;
		isfloat = false;
	}

	LuaValue(Float64 value)
	{
		tag = LUA_TNUMBER;
		number = value;
		isfloat = true;
	}

	LuaValue(const String& value)
	{
		tag = LUA_TSTRING;
		str = value;
		isfloat = false;
	}

	LuaValue(LuaTablePtr t)
	{
		tag = LUA_TTABLE;
		table = t;
		isfloat = false;
	}
};

namespace std
{
	template<> struct hash<LuaValue>
	{
		std::size_t operator()(const LuaValue& value) const noexcept { return value.Hash(); }
	};
}

inline bool ConvertToBoolean(const LuaValue& value)
{
	switch (value.tag)
	{
		case LUA_TNIL: return false;
		case LUA_TBOOLEAN: return value.boolean;
		default: return true;
	}
}

inline std::tuple<Float64, bool> ConvertToFloat(const LuaValue& val)
{
	switch (val.tag)
	{
		case LUA_TNUMBER:
		{
			if(val.isfloat)
				return std::make_tuple(val.number, true);
			else
				return std::make_tuple((Float64)val.integer, true);
		}
		case LUA_TSTRING: return ParseFloat(val.str);
		default: return std::make_tuple(0, false);
	}
}

inline std::tuple<Int64, bool> ConvertToInteger(const LuaValue& val)
{
	switch (val.tag)
	{
		case LUA_TNUMBER:
		{
			if(val.isfloat)
				return FloatToInteger(val.number);
			else
				return std::make_tuple(val.integer, true);
		}
		case LUA_TSTRING: return StringToInteger(val.str);
		default: return std::make_tuple(0, false);
	}
}