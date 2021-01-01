#pragma once
#include "state/lua_state.h"

int OpenBaseLib(LuaState* ls);

int BasePrint(LuaState* ls);
int BaseAssert(LuaState* ls);
int BaseError(LuaState* ls);
int BaseSelect(LuaState* ls);
int BaseIPairs(LuaState* ls);
int BasePairs(LuaState* ls);
int BaseNext(LuaState* ls);
int BaseLoad(LuaState* ls);
int BaseLoadFile(LuaState* ls);
int BaseDoFile(LuaState* ls);
int BasePCall(LuaState* ls);
int BaseXPCall(LuaState* ls);
int BaseGetMetatable(LuaState* ls);
int BaseSetMetatable(LuaState* ls);
int BaseRawEqual(LuaState* ls);
int BaseRawLen(LuaState* ls);
int BaseRawGet(LuaState* ls);
int BaseRawSet(LuaState* ls);
int BaseType(LuaState* ls);
int BaseToString(LuaState* ls);
int BaseToNumber(LuaState* ls);

static const FuncReg BaseFuncs[]
{
	{"print", BasePrint},
	{"assert", BaseAssert},
	{"error", BaseError},
	{"select", BaseSelect},
	{"ipairs", BaseIPairs},
	{"pairs", BasePairs},
	{"next", BaseNext},
	{"load", BaseLoad},
	{"loadfile", BaseLoadFile},
	{"dofile", BaseDoFile},
	{"pcall", BasePCall},
	{"xpcall", BaseXPCall},
	{"getmetatable", BaseGetMetatable},
	{"setmetatable", BaseSetMetatable},
	{"rawequal", BaseRawEqual},
	{"rawlen", BaseRawLen},
	{"rawget", BaseRawGet},
	{"rawset", BaseRawSet},
	{"type", BaseType},
	{"tostring", BaseToString},
	{"tonumber", BaseToNumber},
	/* placeholders */
	{"_G", nullptr},
	{"_VERSION", nullptr},
	{nullptr, nullptr}
};