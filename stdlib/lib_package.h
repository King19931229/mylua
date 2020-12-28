#pragma once
#include "state/lua_state.h"

#define LUA_LOADED_TABLE "_LOADED"
#define LUA_PRELOAD_TABLE "_PRELOAD"

#ifdef _WIN32
#define LUA_DIRSEP "\\"
#else
#define LUA_DIRSEP "/"
#endif
#define LUA_PATH_SEP  ";"
#define LUA_PATH_MARK "?"
#define LUA_EXEC_DIR "!"
#define LUA_IGMARK "-"

int PkgRequire(LuaState* ls);
int PkgSearchPath(LuaState* ls);

static const FuncReg LibFuncs[]
{
	{"require", PkgRequire},
	{nullptr, nullptr}
};

static const FuncReg PkgFuncs[]
{
	{"searchpath", PkgSearchPath},
	{nullptr, nullptr}
};

int OpenPackageLib(LuaState* ls);
void CreateSearchersTable(LuaState* ls);

int PreloadSearcher(LuaState* ls);
int LuaSearcher(LuaState* ls);

std::tuple<String, String> _SearchPath(const String& name, const String& path, const String& sep, const String& dirsep);
void _FindLoader(LuaState* ls, const String& name);