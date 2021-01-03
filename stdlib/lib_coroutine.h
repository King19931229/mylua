#pragma once
#include "state/lua_state.h"

int CoCreate(LuaState* ls);
int CoResume(LuaState* ls);
int CoYield(LuaState* ls);
int CoStatus(LuaState* ls);
int CoYieldable(LuaState* ls);
int CoRunning(LuaState* ls);
int CoWrap(LuaState* ls);

static const FuncReg CoFuncs[]
{
	{"create", CoCreate},
	{"resume", CoResume},
	{"yield", CoYield},
	{"status", CoStatus},
	{"isyieldable", CoYieldable},
	{"running", CoRunning},
	{"wrap", CoWrap},
	{nullptr, nullptr}
};

int OpenCoroutineLib(LuaState* ls);