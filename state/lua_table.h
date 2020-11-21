#pragma once
#include "lua_value.h"
#include <vector>
#include <map>
#include <cmath>

struct LuaTable
{
	std::vector<LuaValue> arr;
	std::unordered_map<LuaValue, LuaValue> map;

	static LuaValue _FloatToInteger(const LuaValue& v)
	{
		if(v.IsFloat64())
		{
			auto pair = FloatToInteger(v.number);
			if(std::get<1>(pair))
			{
				return LuaValue(std::get<0>(pair));
			}
		}
		return v;
	}

	const LuaValue& _GetFromMap(const LuaValue& key) const
	{
		auto it = map.find(key);
		if(it != map.end())
			return it->second;
		return LuaValue::Nil;
	}

	void _EraseKeyFromMap(const LuaValue& key)
	{
		auto it = map.find(key);
		if(it != map.end())
			map.erase(it);
	}

	// Erase the hole in the array
	void _ShrinkArray()
	{
		size_t size = arr.size();
		while(size > 0)
		{
			const LuaValue& val = arr[size - 1];
			if(val != LuaValue::Nil)
				break;
			--size;
		}
		arr.resize(size);
	}

	// Start from the end index of the array to search for Value that
	// can be appended to the array from the Map
	void _ExpandArray()
	{
		size_t idx = arr.size() + 1;
		while(true)
		{
			auto it = map.find(LuaValue((Int64)idx));
			if(it == map.end())
			{
				break;
			}
			else
			{
				LuaValue val = it->second;
				it = map.erase(it);
				arr.push_back(val);
			}
		}
	}

	const LuaValue& Get(const LuaValue& _key) const
	{
		LuaValue key = _FloatToInteger(_key);
		if(key.IsInt64())
		{
			Int64 idx = key.integer;
			if(idx >= 1 && idx <= arr.size())
			{
				return arr[idx - 1];
			}
		}
		return _GetFromMap(key);
	}

	void Put(const LuaValue& _key, const LuaValue& val)
	{
		if(_key == LuaValue::Nil)
			panic("table index is nil!");
		if(_key.IsFloat64() && std::isnan(_key.number))
			panic("table index is NAN!");
		LuaValue key = _FloatToInteger(_key);

		if(key.IsInt64())
		{
			Int64 idx = key.integer;
			if(idx >= 1)
			{
				if(idx <= arr.size())
				{
					arr[idx - 1] = val;
					if(idx == arr.size() && val == LuaValue::Nil)
						_ShrinkArray();
					return;
				}
				else if(idx == arr.size() + 1)
				{
					_EraseKeyFromMap(key);
					if(val != LuaValue::Nil)
					{
						arr.push_back(val);
						_ExpandArray();
					}
					return;
				}
			}
		}

		if(val != LuaValue::Nil)
		{
			map[key] = val;
		}
		else
		{
			_EraseKeyFromMap(key);
		}
	}

	size_t Len() const { return arr.size(); }
};

inline LuaTablePtr NewLuaTable(int nArr, int nRec)
{
	LuaTablePtr t = LuaTablePtr(new LuaTable());
	t->arr.resize(nArr);
	return t;
}