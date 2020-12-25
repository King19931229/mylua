#include "state/lua_state.h"
#include "stdlib/lib_basic.h"

int LuaState::Error2(const char* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	PushFString(fmt, list);
	va_end(list);
	return Error();
}

int LuaState::ArgError(int arg, const String& extraMsg)
{
	return Error2("bad argument #%d (%s)", arg, extraMsg.c_str());
}

void LuaState::CheckStack2(int sz, const String& msg)
{
	if(!CheckStack(sz))
	{
		if(!msg.empty())
			Error2("stack overflow (%s)", msg.c_str());
		else
			Error2("stack overflow");
	}
}

void LuaState::ArgCheck(bool cond, int arg, const String& extraMsg)
{
	if(!cond)
		ArgError(arg, extraMsg);
}

void LuaState::CheckAny(int arg)
{
	if(Type(arg) == LUA_TNONE)
		ArgError(arg, "value expected");
}

void LuaState::CheckType(int arg, LuaType t)
{
	if(Type(arg) != t)
		TagError(arg, t);
}

Int64 LuaState::CheckInteger(int arg)
{
	auto pair = ToIntegerX(arg);
	if(std::get<1>(pair))
	{
		Int64 i = std::get<0>(pair);
		return i;
	}
	IntError(arg);
	return 0;
}

Float64 LuaState::CheckNumber(int arg)
{
	auto pair = ToNumberX(arg);
	if(std::get<1>(pair))
	{
		Float64 f = std::get<0>(pair);
		return f;
	}
	TagError(arg, LUA_TNUMBER);
	return 0;
}

String LuaState::CheckString(int arg)
{
	auto pair = ToStringX(arg);
	if(std::get<1>(pair))
	{
		String s = std::get<0>(pair);
		return s;
	}
	TagError(arg, LUA_TSTRING);
	return "";
}

Int64 LuaState::OptInteger(int arg, Int64 d)
{
	if(IsNoneOrNil(arg))
		return d;
	return CheckInteger(arg);
}

Float64 LuaState::OptNumber(int arg, Float64 d)
{
	if(IsNoneOrNil(arg))
		return d;
	return CheckNumber(arg);
}

String LuaState::OptString(int arg, const String& d)
{
	if(IsNoneOrNil(arg))
		return d;
	return CheckString(arg);
}

bool LuaState::DoFile(const String& filename)
{
	return LoadFile(filename) == LUA_OK && PCall(0, LUA_MULTRET, 0) == LUA_OK;
}

bool LuaState::DoString(const String& str)
{
	return LoadString(str) == LUA_OK && PCall(0, LUA_MULTRET, 0) == LUA_OK;
}

int LuaState::LoadFile(const String& filename)
{
	return LoadFileX(filename, "bt");
}

int LuaState::LoadFileX(const String& filename, const String& mode)
{
	FILE* f = fopen(filename.c_str(), "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		ByteArray chunk;
		chunk.resize(size);
		fread(chunk.data(), 1, size, f);
		fclose(f);
		f = NULL;

		return Load(chunk, "@" + filename, mode);
	}
	return LUA_ERRFILE;
}

int LuaState::LoadString(const String& s)
{
	ByteArray chunk;
	chunk.reserve(s.size());
	for(const Byte& c : s)
	{
		chunk.push_back(c);
	}
	return Load(chunk, s, "bt");
}

String LuaState::TypeName2(int idx)
{
	return TypeName(Type(idx));
}

String LuaState::ToString2(int idx)
{
	if(CallMeta(idx, "__tostring"))
	{
		if(!IsString(-1))
			Error2("\'__totring\' must return a string");
	}
	else
	{
		switch (Type(idx))
		{
			case LUA_TNUMBER:
			{
				if(IsInteger(idx))
					PushString(Format::FormatString("%d", ToInteger(idx)));
				else
					PushString(Format::FormatString("%f", ToNumber(idx)));
				break;				
			}
			case LUA_TSTRING:
			{
				PushValue(idx);
				break;
			}
			case LUA_TBOOLEAN:
			{
				if(ToBoolean(idx))
					PushString("true");
				else
					PushString("false");
				break;
			}
			case LUA_TNIL:
			{
				PushString("nil");
				break;
			}
			default:
			{
				/* try name */
				LuaType tt = GetMetafield(idx, "__name");
				String kind;
				/* get the '__name' as typename */
				if(tt == LUA_TSTRING)
				{
					kind = CheckString(-1);
				}
				/* get the typename */
				else
				{
					kind = TypeName2(idx);
				}
				PushString(Format::FormatString("%s : 0x%x", kind.c_str(), (size_t)ToPointer(idx)));
				if(tt != LUA_TNIL)
				{
					/* remove '__name' */
					Remove(-2);
				}
				break;
			}
		}
	}
	return CheckString(-1);
}

Int64 LuaState::Len2(int idx)
{
	Len(idx);
	auto pair = ToIntegerX(-1);
	Int64 i = 0;
	if(!std::get<1>(pair))
	{
		Error2("object length is not an integer");	
	}
	else
	{
		i = std::get<0>(pair);
	}
	Pop(1);
	return i;	
}

bool LuaState::GetSubTable(int idx, const String& fname)
{
	if(GetField(idx, fname) == LUA_TTABLE)
	{
		/* table already there */
		return true;
	}
	 /* remove previous result */
	Pop(1);
	/* abs index is necessary */
	idx = stack->AbsIndex(idx);
	NewTable();
	/* copy to be left at top */
	PushValue(-1);
	/* assign new table to field */
	SetField(idx, fname);
	/* false, because did not find table there */
	return false;
}

LuaType LuaState::GetMetafield(int obj, const String& e)
{
	/* no metatable? */
	if(!GetMetatable(obj))
	{
		return LUA_TNIL;
	}

	PushString(e);
	LuaType tt = RawGet(-2);
	/* is metafield nil? */
	if(tt == LUA_TNIL)
	{
		/* remove metatable and metafield */
		Pop(2);
	}
	else
	{
		/* remove only metatable */
		Remove(-2);
	}

	return tt;
}

bool LuaState::CallMeta(int obj, const String& e)
{
	obj = AbsIndex(obj);
	/* no metafield? */
	if(GetMetafield(obj, e) == LUA_TNIL)
	{
		return false;
	}

	PushValue(obj);
	Call(1, 1);
	return true;
}

void LuaState::OpenLibs()
{
	FuncReg libs[] = {
		{"_G", OpenBaseLib},
		{nullptr, nullptr}
	};

	FuncReg* lib = libs;
	while(lib && lib->name)
	{
		RequireF(lib->name, lib->func, true);
		Pop(1);
		++lib;
	}
}

void LuaState::RequireF(const String& modname, CFunction openf, bool glb)
{
	GetSubTable(LUA_REGISTRYINDEX, "_LOADED");
	/* LOADED[modname] */
	GetField(-1, modname);
	/* package not already loaded? */
	if(!ToBoolean(-1))
	{
		/* remove field */
		Pop(1);
		PushCFunction(openf);
		/* argument to open function */
		PushString(modname);
		/* call 'openf' to open module */
		Call(1, 1);
		/* make copy of module (call result) */
		PushValue(-1);
		/* _LOADED[modname] = module */
		SetField(-3, modname);
	}
	/* remove _LOADED table */
	Remove(-2);
	if(glb)
	{
		PushValue(-1);
		SetGlobal(modname);
	}
}

void LuaState::NewLib(const FuncReg* l)
{
	NewLibTable(l);
	SetFuncs(l, 0);
}

void LuaState::NewLibTable(const FuncReg* l)
{
	const FuncReg* lib = l;
	int nRec = 0;
	while(lib && lib->name)
	{
		++lib;
		++nRec;
	}
	CreateTable(0, nRec);
}

void LuaState::SetFuncs(const FuncReg* l, int nup)
{
	CheckStack2(nup, "too many upvalues");
	const FuncReg* lib = l;
	/* fill the table with given functions */
	while(lib && lib->name)
	{
		/* copy upvalues to the top */
		for(int i = 0; i < nup; ++i)
			PushValue(-nup);
		/* closure with those upvalues */
		PushCClosure(lib->func, nup);
		// r[-(nup+2)][name]=fun
		// -(nup+1+1) +1 because already push closure
		// a table below upvalues
		SetField(-(nup + 2), lib->name);

		++lib;
	}
	/* remove upvalues */	
	Pop(nup);
}