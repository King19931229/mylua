#pragma once
#include "public.h"

enum LuaType
{
	LUA_TNONE = -1,
	LUA_TNIL,
	LUA_TBOOLEAN,
	LUA_TLIGHTUSERDATA,
	LUA_TNUMBER,
	LUA_TSTRING,
	LUA_TTABLE,
	LUA_TFUNCTION,
	LUA_TUSERDATA,
	LUA_TTHREAD
};

inline String TypeName(LuaType tp)
{
	switch (tp)
	{
		case LUA_TNONE: return "no value";
		case LUA_TNIL: return "nil";
		case LUA_TBOOLEAN: return "boolean";
		case LUA_TNUMBER: return "number";
		case LUA_TSTRING: return "string";
		case LUA_TTABLE: return "table";
		case LUA_TFUNCTION: return "function";	
		case LUA_TTHREAD: return "thread";
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		default: return "userdata";
	}
}

static constexpr int LUA_MINSTACK = 20;
static constexpr int LUAI_MAXSTACK = 1000000;
static constexpr int LUA_REGISTRYINDEX = -LUAI_MAXSTACK - 1000;
static constexpr Int64 LUA_RIDX_GLOBALS = 2;