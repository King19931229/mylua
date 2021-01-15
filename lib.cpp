#include "stdlib/lib_basic.h"
#include "stdlib/lib_package.h"
#include "stdlib/lib_coroutine.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <direct.h>
#include <windows.h>
#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define ACCESS(path, mode) _access(path, mode)
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#ifdef _MSC_VER
#define F_OK	0	/* Check for file existence */
#define X_OK	1	/* MS access() doesn't check for execute permission. */
#define W_OK	2	/* Check for write permission */
#define R_OK	4	/* Check for read permission */
#endif
#else
#define ACCESS(path, mode) access(path, mode)
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define RMDIR(path) rmdir(path)
#endif

// Base Library

int OpenBaseLib(LuaState* ls)
{
	/* open lib into global table */
	ls->PushGlobalTable();
	ls->SetFuncs(BaseFuncs, 0);
	/* set global _G */
	ls->PushValue(-1);
	ls->SetField(-2, "_G");
	/* set global _VERSION */
	ls->PushString("Lua 5.3");
	ls->SetField(-2, "_VERSION");
	return 1;
}

int BasePrint(LuaState* ls)
{
	int n = ls->GetTop(); /* number of arguments */
	ls->GetGlobal("tostring");
	for (int i = 1; i <= n; i++)
	{
		/* function to be called */
		ls->PushValue(-1);
		/* value to print */
		ls->PushValue(i);
		ls->Call(1, 1);
		/* get result */
		auto pair = ls->ToStringX(-1);
		if (!std::get<1>(pair))
		{
			return ls->Error2("'tostring' must return a string to 'print'");
		}
		if (i > 1)
		{
			printf("\t");
		}
		const String& s = std::get<0>(pair);
		printf("%s", s.c_str());
		/* pop result */
		ls->Pop(1);
	}
	puts("");
	return 0;
}

int BaseAssert(LuaState* ls)
{
	/* condition is true? */
	if(ls->ToBoolean(1))
	{
		/* return all arguments */
		return ls->GetTop();
	}
	else
	{
		/* there must be a condition */
		ls->CheckAny(1);
		/* remove it */
		ls->Remove(1);
		/* default message */
		ls->PushString("assertion failed!");
		/* leave only message (default if no other one) */
		ls->SetTop(1);
		/* call 'error' */
		return BaseError(ls);
	}
}

int BaseError(LuaState* ls)
{
	Int64 level = ls->OptInteger(2, 1);
	ls->SetTop(1);
	if(ls->Type(1) == LUA_TSTRING && level > 0)
	{
		/* add extra information */
		// ls->Where(level);
		// ls->PushValue(1)
		// ls->Concat(2);
	}
	return ls->Error();
}

int BaseSelect(LuaState* ls)
{
	Int64 n = ls->GetTop();
	if(ls->Type(1) == LUA_TSTRING && ls->CheckString(1) == "#")
	{
		ls->PushInteger(n - 1);
		return 1;
	}
	else
	{
		Int64 i = ls->CheckInteger(1);
		// i will be the index between [1, n - 1]
		if(i < 0)
			// n - 1 + i + 1
			i = n + i;
		else if(i > n)
			i = n;
		// i == 0 will return all arguments(index, ...)
		// i == n will return nothing
		ls->ArgCheck(1 <= i, 1, "index out of range");
		return (int)(n - i);
	}
}

static int iPairsAux(LuaState* ls)
{
	Int64 i = ls->ToInteger(2) + 1;
	ls->PushInteger(i);
	if(ls->GetI(1, i) == LUA_TNIL)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int BaseIPairs(LuaState* ls)
{
	ls->CheckAny(1);
	/* iteration function */
	ls->PushCFunction(iPairsAux);
	/* state */
	ls->PushValue(1);
	/* initial value */
	ls->PushInteger(0);
	return 3;
}

int BasePairs(LuaState* ls)
{
	ls->CheckAny(1);
	/* no metamethod? */
	if(ls->GetMetafield(1, "__pairs") == LUA_TNIL)
	{
		/* will return generator, */
		ls->PushCFunction(BaseNext);
		ls->PushValue(1);
		/* state, */
		ls->PushNil();
	}
	else
	{
		/* argument 'self' to metamethod */
		ls->PushValue(1);
		/* get 3 values from metamethod */
		ls->Call(1, 3);
	}
	return 3;
}

int BaseNext(LuaState* ls)
{
	ls->CheckType(1, LUA_TTABLE);
	/* create a 2nd argument if there isn't one */
	ls->SetTop(2);
	if (ls->Next(1))
	{
		return 2;
	}
	else
	{
		ls->PushNil();
		return 1;
	}
}

int LoadAux(LuaState* ls, int status, int envIdx)
{
	if(status == LUA_OK)
	{
		if(envIdx != 0)
		{
			/* 'env' parameter? */
			panic("todo!");
		}
		return 1;
	}
	else
	{
		/* error (message is on top of the stack) */
		ls->PushNil();
		/* put before error message */
		ls->Insert(-2);
		return 2;
	}
}

int BaseLoad(LuaState* ls)
{
	int status = 0;
	auto strResult = ls->ToStringX(1);
	const String& chunk = std::get<0>(strResult);
	bool isStr = std::get<1>(strResult);
	String mode = ls->OptString(3, "bt");
	/* 'env' index or 0 if not 'env' */
	int env = 0;
	if(!ls->IsNone(4))
	{
		env = 4;
	}
	if(isStr)
	{
		/* loading a string? */
		String chunkName = ls->OptString(2, chunk);

		ByteArray chunkByte;
		chunkByte.reserve(chunk.size());
		for(const Byte& c : chunk)
		{
			chunkByte.push_back(c);
		}
		status = ls->Load(chunkByte, chunkName, mode);
	}
	else
	{
		/* loading from a reader function */
		panic("loading from a reader function"); // todo
	}
	return LoadAux(ls, status, env);
}

int BaseLoadFile(LuaState* ls)
{
	String fname = ls->OptString(1, "");
	String mode = ls->OptString(2, "bt");
	/* 'env' index or 0 if not 'env' */
	int env = 0;
	if(!ls->IsNone(3))
	{
		env = 3;
	}
	int status = ls->LoadFileX(fname, mode);
	return LoadAux(ls, status, env);
}

int BaseDoFile(LuaState* ls)
{
	String fname = ls->OptString(1, "bt");
	ls->SetTop(1);
	if(ls->LoadFile(fname) != LUA_OK)
	{
		return ls->Error();
	}
	ls->Call(0, LUA_MULTRET);
	return ls->GetTop() - 1;
}

int BasePCall(LuaState* ls)
{
	int nArgs = ls->GetTop() - 1;
	int status = ls->PCall(nArgs, -1,  0);
	ls->PushBoolean(status == LUA_OK);
	ls->Insert(1);
	return ls->GetTop();
}

int BaseXPCall(LuaState* ls)
{
	panic("todo");
	return 0;
}

int BaseGetMetatable(LuaState* ls)
{
	ls->CheckAny(1);
	if(!ls->GetMetatable(1))
	{
		ls->PushNil();
		/* no metatable */
		return 1;
	}
	ls->GetMetafield(1, "__metatable");
	/* returns either __metatable field (if present) or metatable */
	return 1;
}

int BaseSetMetatable(LuaState* ls)
{
	LuaType t = ls->Type(2);
	ls->CheckType(1, LUA_TTABLE);
	ls->ArgCheck(t == LUA_TNIL || t == LUA_TTABLE, 2,
		"nil or table expected");
	if(ls->GetMetafield(1, "__metatable") != LUA_TNIL)
	{
		ls->Error2("cannot change a protected metatable");
	}
	ls->SetTop(2);
	ls->SetMetatable(1);
	return 1;
}

int BaseRawEqual(LuaState* ls)
{
	ls->CheckAny(1);
	ls->CheckAny(2);
	ls->PushBoolean(ls->RawEqual(1, 2));
	return 1;
}

int BaseRawLen(LuaState* ls)
{
	LuaType t = ls->Type(1);
	ls->ArgCheck(t == LUA_TTABLE || t == LUA_TSTRING, 1,
		"table or string expected");
	ls->PushInteger(Int64(ls->RawLen(1)));
	return 1;
}

int BaseRawGet(LuaState* ls)
{
	ls->CheckType(1, LUA_TTABLE);
	ls->CheckAny(2);
	ls->SetTop(2);
	ls->RawGet(1);
	return 1;
}

int BaseRawSet(LuaState* ls)
{
	ls->CheckType(1, LUA_TTABLE);
	ls->CheckAny(2);
	ls->CheckAny(3);
	ls->SetTop(3);
	ls->RawSet(1);
	return 1;
}

int BaseType(LuaState* ls)
{
	LuaType t = ls->Type(1);
	ls->ArgCheck(t != LUA_TNONE, 1, "value excepted");
	ls->PushString(ls->TypeName(t));
	return 1;
}

int BaseToString(LuaState* ls)
{
	ls->CheckAny(1);
	ls->ToString2(1);
	return 1;
}

int BaseToNumber(LuaState* ls)
{
	/* standard conversion? */
	if(ls->IsNoneOrNil(2))
	{
		ls->CheckAny(1);
		/* already a number? */
		if(ls->Type(1) == LUA_TNUMBER)
		{
			/* yes; return it */
			ls->SetTop(1);
			return 1;
		}
		else
		{
			auto pair = ls->ToStringX(1);
			if(std::get<1>(pair))
			{
				const String& s = std::get<0>(pair);
				if(ls->StringToNumber(s))
				{
					/* successful conversion to number */
					return 1;
				}/* else not a number */
			}
		}
	}
	else
	{
		/* no numbers as strings */
		ls->CheckType(1, LUA_TSTRING);
		// todo TrimSpace
		String s = ls->ToString(1);
		int base = (int)ls->CheckInteger(2);
		ls->ArgCheck(2 <= base && base <= 36, 2, "base out of range");
		// todo turn s into base number strconv.ParseInt(s, base)
		if(false)
		{
			ls->PushInteger(0);
			return 1;
		}/* else not a number */
	}/* else not a number */
	ls->PushNil();
	return 1;
}

// Package Library

int OpenPackageLib(LuaState* ls)
{
	/* create 'package' table */
	ls->NewLib(PkgFuncs);
	CreateSearchersTable(ls);
	/* set paths */
	ls->PushString("./?.lua;./?/init.lua");
	ls->SetField(-2, "path");
	/* store config information */
	ls->PushString(String(LUA_DIRSEP) + "\n" + LUA_PATH_SEP + "\n"
		+ LUA_PATH_MARK + "\n" + LUA_EXEC_DIR + "\n" + LUA_IGMARK + "\n");
	ls->SetField(-2, "config");
	/* set field 'loaded' */
	ls->GetSubTable(LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
	ls->SetField(2, "loaded");
	/* set field 'preload' */
	ls->GetSubTable(LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
	ls->SetField(2, "preload");
	ls->PushGlobalTable();
	/* set 'package' as upvalue for next lib*/
	ls->PushValue(-2);
	/* open lib into global table */
	ls->SetFuncs(LibFuncs, 1);
	/* pop global table */
	ls->Pop(1);
	/* return 'package' table */
	return 1;
}

void CreateSearchersTable(LuaState* ls)
{
	static constexpr CFunction searchers[]
	{
		PreloadSearcher,
		LuaSearcher
	};
	static constexpr int searchersLen = sizeof(searchers) / sizeof(searchers[0]);
	/* create 'searchers' table */
	ls->CreateTable(searchersLen, 0);
	/* fill it with predefined searchers */
	for(int idx = 0; idx < searchersLen; ++idx)
	{
		/* set 'package' as upvalue for all searchers */
		ls->PushValue(-2);
		ls->PushCClosure(searchers[idx], 1);
		/* searchers[idx + 1] = closure */
		ls->RawSetI(-2, Int64(idx + 1));
	}
	/* put it in field 'searchers' */
	/* package[searchers] = searchers */
	ls->SetField(-2, "searchers");
}

int PreloadSearcher(LuaState* ls)
{
	const String& name = ls->CheckString(1);
	ls->GetField(LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
	if(ls->GetField(-1, name) == LUA_TNIL)
	{
		/* not found */
		ls->PushString(Format::FormatString("\n\tno field package.preload[\"%s\"]", name.c_str()));
	}
	return 1;
}

int LuaSearcher(LuaState* ls)
{
	const String& name = ls->CheckString(1);
	/* see CreateSearchersTable, package table is the upvalue. */
	ls->GetField(LuaUpvalueIndex(1), "path");
	auto pathRes = ls->ToStringX(-1);
	if(!std::get<1>(pathRes))
	{
		ls->Error2("'package.path' must be a string");
	}
	const String& path = std::get<0>(pathRes);
	auto searchRes = _SearchPath(name, path, ".", LUA_DIRSEP);
	const String& filename = std::get<0>(searchRes);
	const String& errMsg = std::get<1>(searchRes);
	if(errMsg != "")
	{
		ls->PushString(errMsg);
		return 1;
	}
	/* module loaded successfully? */
	if(ls->LoadFile(filename) == LUA_OK)
	{
		/* will be 2nd argument to module */
		ls->PushString(filename);
		return 2;
	}
	else
	{
		return ls->Error2("error loading module '%s' from file '%s':\n\t%s",
			ls->CheckString(1).c_str(), filename.c_str(), ls->CheckString(-1).c_str());
	}
}

std::tuple<String, String> _SearchPath(const String& _name, const String& path, const String& sep, const String& dirsep)
{
	String name = _name;
	if(sep != "")
	{
		name = StringUtil::Replace(name, sep, dirsep);
	}
	String errMsg;
	StringArray splitRes = StringUtil::Split(path, LUA_PATH_SEP);
	for(const String& res : splitRes)
	{
		String filename = StringUtil::Replace(res, LUA_PATH_MARK, name);
		if(!ACCESS(filename.c_str(), W_OK))
		{
			return std::make_pair(filename, "");
		}
		errMsg += Format::FormatString("\n\t no file '%s'", filename.c_str());
	}
	return std::make_pair("", errMsg);
}

int PkgSearchPath(LuaState* ls)
{
	String name = ls->CheckString(1);
	String path = ls->CheckString(2);
	String sep = ls->OptString(3, ".");
	String rep = ls->OptString(4, LUA_DIRSEP);
	auto searchRes = _SearchPath(name, path, sep, rep);
	const String& filename = std::get<0>(searchRes);
	const String& errMsg = std::get<1>(searchRes);
	if(errMsg == "")
	{
		ls->PushString(filename);
		return 1;
	}
	else
	{
		ls->PushNil();
		ls->PushString(errMsg);
		return 2;
	}
}

int PkgRequire(LuaState* ls)
{
	String name = ls->CheckString(1);
	/* LOADED table will be at index 2 */
	ls->SetTop(1);
	ls->GetField(LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
	/* LOADED[name] */
	ls->GetField(2, name);
	/* is it there? */
	if(ls->ToBoolean(-1))
	{
		/* package is already loaded */
		return 1;
	}
	/* else must load package */
	/* remove 'getfield' result */
	ls->Pop(1);
	_FindLoader(ls, name);
	/* pass name as argument to module loader */
	ls->PushString(name);
	/* name is 1st argument (before search data) */
	ls->Insert(-2);
	/* run loader to load module */
	ls->Call(2, 1);
	/* non-nil return ? */
	if(!ls->IsNil(-1))
	{
		/* LOADED[name] = returned value */
		ls->SetField(2, name);
	}
	/* module set no value? */
	if(ls->GetField(2, name) == LUA_TNIL)
	{
		/* use true as result */
		ls->PushBoolean(true);
		/* extra copy to be returned */
		ls->PushValue(-1);
		/* LOADED[name] = true */
		ls->SetField(2, name);
	}
	return 1;
}

void _FindLoader(LuaState* ls, const String& name)
{
	/* push 'package.searchers' to index 3 in the stack */
	if(ls->GetField(LuaUpvalueIndex(1), "searchers") != LUA_TTABLE)
	{
		ls->Error2("'package.searchers' must be a table");
	}

	/* to build error message */
	String errMsg = "module '" + name + "' not found:";

	/* iterate over available searchers to find a loader */
	for(Int64 i = 1;;++i)
	{
		/* no more searchers */
		if(ls->RawGetI(3, i) == LUA_TNIL)
		{
			/* remove nil */
			ls->Pop(1);
			/* create error message */
			ls->Error2(errMsg.c_str());
			return;
		}

		ls->PushString(name);
		/* call it */
		ls->Call(1, 2);
		/* did it find a loader? */
		if(ls->IsFunction(-2))
		{
			/* module loader found */
			return;
		}
		else if(ls->IsString(-2))
		{
			/* remove extra return */
			ls->Pop(1);
			/* concatenate error message */
			errMsg += ls->CheckString(-1);
		}
		else
		{
			/* remove both returns */
			ls->Pop(2);
		}
	}
}

/* Coroutine */
int OpenCoroutineLib(LuaState* ls)
{
	ls->NewLib(CoFuncs);
	return 1;
}

int CoCreate(LuaState* ls)
{
	ls->CheckType(1, LUA_TFUNCTION);
	LuaStatePtr ls2 = ls->NewThread();
	/* move function to top */
	ls->PushValue(1);
	/* move function from ls to ls2 */
	ls->XMove(ls2.get(), 1);
	return 1;
}

int _AuxResume(LuaState* ls, LuaStatePtr co, int narg)
{
	if(!ls->CheckStack(narg))
	{
		ls->PushString("too many arguments to resume");
		/* error flag */
		return -1;
	}
	if(co->Status() == LUA_OK && co->GetTop() == 0)
	{
		ls->PushString("cannot resume dead coroutine");
		/* error flag */
		return -1;
	}
	ls->XMove(co.get(), narg);
	int status = co->Resume(ls, narg);
	if(status == LUA_OK || status == LUA_YIELD)
	{
		int nres = co->GetTop();
		if(!ls->CheckStack(nres + 1))
		{
			/* remove results anyway */
			co->Pop(nres);
			ls->PushString("too many yielded values");
			/* error flag */
			return -1;
		}
		co->XMove(ls, nres);
		return nres;
	}
	else
	{
		/* move error message */
		co->XMove(ls, 1);
		return -1;
	}
}

int CoResume(LuaState* ls)
{
	LuaStatePtr co = ls->ToThread(1);
	ls->ArgCheck(co != nullptr, 1, "thread expected");

	int r = _AuxResume(ls, co, ls->GetTop() - 1);
	if(r < 0)
	{
		ls->PushBoolean(false);
		ls->Insert(-2);
		/* return false + error message */
		return 2;
	}
	else
	{
		ls->PushBoolean(true);
		ls->Insert(-(r + 1));
		return r + 1;
	}
}

#ifdef Yield
#	undef Yield
#endif

int CoYield(LuaState* ls)
{
	int nResults = ls->GetTop();
	ls->Yield(nResults);
	return nResults;
}

int CoStatus(LuaState* ls)
{
	LuaStatePtr co = ls->ToThread(1);
	ls->ArgCheck(co != nullptr, 1, "thread expected");
	if(ls == co.get())
	{
		ls->PushString("running");
	}
	else
	{
		switch(co->Status())
		{
			case LUA_YIELD:
				ls->PushString("suspended");
				break;
			case LUA_OK:
				/* does it have frames */
				if(co->GetStack())
				{
					/* it is running */
					ls->PushString("normal");
				}
				else if(co->GetTop() == 0)
				{
					ls->PushString("dead");
				}
				else
				{
					ls->PushString("suspended");
				}
				break;
			default:
				/* some error occurred */
				ls->PushString("dead");
				break;
		}
	}
	return 1;
}

int CoYieldable(LuaState* ls)
{
	ls->PushBoolean(ls->IsYieldable());
	return 1;
}

int CoRunning(LuaState* ls)
{
	bool isMain = ls->PushThread();
	ls->PushBoolean(isMain);
	return 2;
}

int CoWrap(LuaState* ls)
{
	panic("todo: coWrap!");
	return 0;
}