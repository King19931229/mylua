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
static constexpr int LFIELDS_PER_FLUSH = 50;
static constexpr int LUA_MULTRET = -1;

static constexpr int LUA_OK			= 0;
static constexpr int LUA_YIELD		= 1;
static constexpr int LUA_ERRRUN		= 2;
static constexpr int LUA_ERRSYNTAX	= 3;
static constexpr int LUA_ERRMEM		= 4;
static constexpr int LUA_ERRGCMM	= 5;
static constexpr int LUA_ERRERR		= 6;
static constexpr int LUA_ERRFILE	= 7;

inline int LuaUpvalueIndex(int i) { return LUA_REGISTRYINDEX - i; }