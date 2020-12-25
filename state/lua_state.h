#pragma once
#include "lua_stack.h"
#include "api_arith.h"
#include "api_compare.h"
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "compiler/compiler.h"

enum ArithOp
{
	LUA_OPADD, // +
	LUA_OPSUB, // -
	LUA_OPMUL, // *
	LUA_OPMOD, // %
	LUA_OPPOW, // ^
	LUA_OPDIV, // /
	LUA_OPIDIV, // //
	LUA_OPBAND, // &
	LUA_OPBOR, // |
	LUA_OPBXOR, // ~
	LUA_OPSHL, // <<
	LUA_OPSHR, // >>
	LUA_OPUNM, // - (unary minus)
	LUA_OPBNOT, // ~
};

enum CompareOp
{
	LUA_OPEQ,
	LUA_OPLT,
	LUA_OPLE,
};

void PrintStack(LuaState& state);

struct LuaState
{
	LuaStackPtr stack;
	LuaTablePtr registry;

	int GetTop() const;
	int AbsIndex(int idx) const;
	bool CheckStack(int n);
	void Pop(int n);
	void Copy(int fromIdx, int toIdx);
	void PushValue(int idx);
	void Replace(int idx);
	void Rotate(int idx, int n);
	void SetTop(int idx);
	void Insert(int idx);
	void Remove(int idx);
	void PushNil();
	void PushBoolean(bool b);
	void PushInteger(Int64 n);
	void PushNumber(Float64 n);
	void PushString(const String& str);
	LuaType Type(int idx) const;
	bool IsNone(int idx) const;
	bool IsNil(int idx) const;
	bool IsNoneOrNil(int idx) const;
	bool IsBoolean(int idx) const;
	bool IsInteger(int idx) const;
	bool IsNumber(int idx) const;
	bool IsString(int idx) const;
	bool IsTable(int idx) const;
	bool IsThread(int idx) const;
	bool IsFunction(int idx) const;
	void* ToPointer(int idx) const;
	bool ToBoolean(int idx) const;
	std::tuple<Float64, bool> ToNumberX(int idx) const;
	Float64 ToNumber(int idx) const;
	std::tuple<Int64, bool> ToIntegerX(int idx) const;
	Int64 ToInteger(int idx) const;
	std::tuple<String, bool> ToStringX(int idx);
	String ToString(int idx);
	void Arith(ArithOp op);
	bool Compare(int idx1, int idx2, CompareOp op);
	bool RawEqual(int idx1, int idx2);
	void Len(int idx);
	void RawLen(int idx);
	void Concat(int n);
	void PushFString(const char* fmt, ...);
	/*
	interfaces for table
	*/
	void CreateTable(int nArr, int nRec);
	void NewTable();
	LuaType _GetTable(const LuaValue& t, const LuaValue& k, bool raw);
	LuaType GetTable(int idx);
	LuaType GetField(int idx, const String& k);
	LuaType RawGetField(int idx, const String& k);
	LuaType GetI(int idx, Int64 k);
	LuaType RawGet(int idx);
	LuaType RawGetI(int idx, Int64 k);
	void _SetTable(const LuaValue& t, const LuaValue& k, const LuaValue& v, bool raw);
	void SetTable(int idx);
	void SetField(int idx, const String& k);
	void SetI(int idx, Int64 i);
	void RawSetI(int idx, Int64 i);
	void PushLuaStack(LuaStackPtr s);
	void PopLuaStack();
	bool IsBinaryChunk(const ByteArray& chunk);
	int Load(const ByteArray& chunk, const String& chunkName, const String& mode);
	void RunLuaClosure();
	void CallLuaClosure(int nArgs, int nResults, ClosurePtr c);
	void CallCClosure(int nArgs, int nResults, ClosurePtr c);
	void Call(int nArgs, int nResults);
	void PushCClosure(CFunction c, int n);
	void PushCFunction(CFunction c);
	bool IsCFunction(int idx);
	CFunction ToCFunction(int idx);
	void PushGlobalTable();
	LuaType GetGlobal(const String& name);
	void SetGlobal(const String& name);
	void Register(const String& name, CFunction f);
	bool GetMetatable(int idx);
	void SetMetatable(int idx);
	bool Next(int idx);
	int Error();
	int PCall(int nArgs, int nResults, int msgh);
	/*
	interfaces for luavm
	*/
	int PC() const;
	void AddPC(int n);
	UInt32 Fetch();
	void GetConst(int idx);
	void GetRK(int rk);
	int RegisterCount() const;
	void LoadVararg(int n);
	void LoadProto(int idx);
	void CloseUpvalues(int a);
	/*
	interfaces for auxlib
	*/
	int Error2(const char* fmt, ...);
	int ArgError(int arg, const String& extraMsg);
	void CheckStack2(int sz, const String& msg);
	void ArgCheck(bool cond, int arg, const String& extraMsg);
	void CheckAny(int arg);
	void CheckType(int arg, LuaType t);
	Int64 CheckInteger(int arg);
	Float64 CheckNumber(int arg);
	String CheckString(int arg);
	Int64 OptInteger(int arg, Int64 d);
	Float64 OptNumber(int arg, Float64 d);
	String OptString(int arg, const String& d);
	/* Load functions */
	bool DoFile(const String& filename);
	bool DoString(const String& str);
	int LoadFile(const String& filename);
	int LoadFileX(const String& filename, const String& mode);
	int LoadString(const String& s);
	/* Other functions */
	String TypeName2(int idx);
	String ToString2(int idx);
	Int64 Len2(int idx);
	bool GetSubTable(int idx, const String& fname);
	LuaType GetMetafield(int obj, const String& e);
	bool CallMeta(int obj, const String& e);
	void OpenLibs();
	void RequireF(const String& modname, CFunction openf, bool glb);
	void NewLib(const FuncReg* l);
	void NewLibTable(const FuncReg* l);
	void SetFuncs(const FuncReg* l, int nup);
	/* Error functions*/
	void IntError(int arg);
	void TagError(int arg, LuaType tag);
	int TypeError(int arg, const String& tname);
};

inline LuaStatePtr NewLuaState()
{
	LuaTablePtr registry = NewLuaTable(0, 0);
	LuaValue global = LuaValue(NewLuaTable(0, 0));
	registry->Put(LuaValue(LUA_RIDX_GLOBALS), global);

	LuaStatePtr ls = LuaStatePtr(new LuaState
		{
			nullptr,
			registry
		}
	);

	ls->PushLuaStack(NewLuaStack(LUA_MINSTACK, ls.get()));

	return ls;
}