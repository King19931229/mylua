#pragma once
#include "api/consts.h"
#include "type.h"

struct LuaValue
{
	LuaType tag;
	union
	{
		bool boolean;
		UInt64 integer;
		double number;
	};
	bool isfloat;
	std::string str;

	const static LuaValue NoValue;
	const static LuaValue Nil;

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

	LuaValue(UInt64 value)
	{
		tag = LUA_TNUMBER;
		integer = value;
		isfloat = false;
	}

	LuaValue(double value)
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