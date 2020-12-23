#include "state/lua_state.h"

int LuaState::Error2(const String& fmt, ...)
{
	return 0;
}

int LuaState::ArgError(int arg, const String& extraMsg)
{
	return 0;
}

void LuaState::CheckState2(int sz, const String& msg)
{

}

void LuaState::ArgCheck(bool cond, int arg, const String& extraMsg)
{

}

void LuaState::CheckAny(int arg)
{

}

void LuaState::CheckType(int arg, LuaType t)
{

}

Int64 LuaState::CheckInteger(int arg)
{
	return 0;
}

Float64 LuaState::CheckNumber(int arg)
{
	return 0;
}

const String LuaState::CheckString(int arg)
{
	return "";
}

Int64 LuaState::OptInteger(int arg, Int64 d)
{
	return 0;
}

Float64 LuaState::OptNumber(int arg, Float64 d)
{
	return 0;
}

const String LuaState::OptString(int arg, const String& d)
{
	return "";
}

bool LuaState::DoFile(const String& filename)
{
	return false;
}

bool LuaState::DoString(const String& str)
{
	return false;
}

int LuaState::LoadFile(const String& filename)
{
	return 0;
}

int LuaState::LoadFileX(const String& filename, const String& mode)
{
	return 0;
}

int LuaState::LoadString(const String& s)
{
	return 0;
}

const String LuaState::TypeName2(int idx)
{
	return "";
}

const String LuaState::ToString2(int idx)
{
	return "";
}

Int64 LuaState::Len2(int idx)
{
	return 0;
}

bool LuaState::GetSubTable(int idx, const String& fname)
{
	return false;
}

LuaType LuaState::GetMetafield(int obj, const String& e)
{
	return LUA_TNONE;
}

bool LuaState::CallMeta(int obj, const String& e)
{
	return false;
}

void LuaState::OpenLibs()
{

}

void LuaState::RequireF(const String& modname, CFunction openf, bool glb)
{

}

void LuaState::NewLib(const FuncReg& l)
{

}

void LuaState::NewLibTable(const FuncReg& l)
{

}

void LuaState::SetFuncs(const FuncReg& l, int nup)
{

}