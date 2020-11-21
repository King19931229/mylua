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


static constexpr int LUA_MINSTACK = 20;
static constexpr int LUAI_MAXSTACK = 1000000;
static constexpr int LUA_REGISTRYINDEX = -LUAI_MAXSTACK - 1000;
static constexpr Int64 LUA_RIDX_GLOBALS = 2;