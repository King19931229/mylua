#pragma once
#include "api/consts.h"
#include "number/parser.h"

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

	const static LuaValue NoValue;
	const static LuaValue Nil;

	inline bool IsInt64() const { return tag == LUA_TNUMBER && !isfloat; }
	inline bool IsFloat64() const { return tag == LUA_TNUMBER && isfloat; }
	inline bool IsBool() const { return tag == LUA_TBOOLEAN; }
	inline bool IsString() const { return tag == LUA_TSTRING; }

	bool operator==(const LuaValue& rhs) const
	{
		if(tag != rhs.tag)
			return false;
		// number comprare and integer compare is the same result
		if(tag == LUA_TNUMBER && (number != rhs.number || isfloat != rhs.isfloat))
			return false;
		if(tag == LUA_TBOOLEAN && boolean != rhs.boolean)
			return false;
		if(tag == LUA_TSTRING && str != rhs.str)
			return false;
		return true;
	}

	bool operator!=(const LuaValue& rhs) const { return !(*this == rhs); }

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
};

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