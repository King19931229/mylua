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

	int GetTop() const
	{
		return (int)stack->top;
	}

	int AbsIndex(int idx) const
	{
		return stack->AbsIndex(idx);
	}

	bool CheckStack(int n)
	{
		stack->Check(n);
		return true;
	}

	void Pop(int n)
	{
		for(int i = 0; i < n; ++i)
			stack->Pop();
	}

	void Copy(int fromIdx, int toIdx)
	{
		LuaValue val = stack->Get(fromIdx);
		stack->Set(toIdx, val);
	}

	void PushValue(int idx)
	{
		LuaValue val = stack->Get(idx);
		stack->Push(val);
	}

	// Pop the value from the top, and set it back the stack by the index(so the index should be > 0)
	void Replace(int idx)
	{
		LuaValue val = *stack->Pop();
		stack->Set(idx, val);
	}

	/*
	Push the element of [idx, top] to the right by n units (if n >= 0)
	otherwise push it to the left by n units
	For array [1,2,3,4,5,6,7] move 4 units to the right
	[1,2,3,4,5,6,7] -> [3,2,1,4,5,6,7] -> [3,2,1,7,6,5,4] -> [4,5,6,7,1,2,3]
	It can be seen that p is the starting index,
	t is the ending index, and shifted to the right by n units ==> r(p, t - n), r(t - n + 1, t), r(p, t)
	On the contrary, if [1,2,3,4,5,6,7] move 4 units to the left
	[1,2,3,4,5,6,7] -> [4,3,2,1,5,6,7] -> [4,3,2,1,7,6,5] -> [5,6,7,1,2,3,4]
	It can be seen that p is the starting index,
	t is the ending index, and shifted to the left by n units ==> r(p, p + n - 1), r(p + n, t), r(p, t)
	*/
	void Rotate(int idx, int n)
	{
		size_t t = stack->top - 1;
		size_t p = stack->AbsIndex(idx) - 1;
		size_t m = (n >= 0) ? (t - n) : (p - n - 1);
		stack->_Reverse(p, m);
		stack->_Reverse(m + 1, t);
		stack->_Reverse(p, t);
	}

	// Set capacity of the stack(idx means the capacity)
	void SetTop(int idx)
	{
		size_t newTop = stack->AbsIndex(idx);
		if(newTop < 0)
		{
			panic("stack underflow");
		}
		int n = (int)stack->top - (int)newTop;
		if(n > 0)
		{
			for(int i = 0; i < n; ++i)
				stack->Pop();
		}
		else if(n < 0)
		{
			for(int i = 0; i > n; --i)
				stack->Push(LuaValue::Nil);
		}
	}

	// Pop the top of the stack and insert it into the specified position
	void Insert(int idx)
	{
		Rotate(idx, 1);
	}

	void Remove(int idx)
	{
		Rotate(idx, -1);
		Pop(1);
	}

	void PushNil() { stack->Push(LuaValue::Nil); }
	void PushBoolean(bool b) { stack->Push(LuaValue(b)); }
	void PushInteger(Int64 n) { stack->Push(LuaValue(n)); }
	void PushNumber(Float64 n) { stack->Push(LuaValue(n)); }
	void PushString(const String& str){ stack->Push(LuaValue(str)); }

	LuaType Type(int idx) const
	{
		if(stack->IsValid(idx))
		{
			LuaValue value = stack->Get(idx);
			return value.tag;
		}
		return LUA_TNONE;
	}

	bool IsNone(int idx) const { return Type(idx) == LUA_TNONE; }
	bool IsNil(int idx) const { return Type(idx) == LUA_TNIL; }
	bool IsNoneOrNil(int idx) const { return Type(idx) <= LUA_TNIL; }
	bool IsBoolean(int idx) const { return Type(idx) == LUA_TBOOLEAN; }
	bool IsString(int idx) const
	{
		LuaType t = Type(idx);
		return t == LUA_TSTRING || t == LUA_TNUMBER;
	}

	bool ToBoolean(int idx) const
	{
		LuaValue value = stack->Get(idx);
		return ConvertToBoolean(value);
	}

	std::tuple<Float64, bool> ToNumberX(int idx) const
	{
		LuaValue value = stack->Get(idx);
		return ConvertToFloat(value);
	}

	Float64 ToNumber(int idx) const
	{
		auto pair = ToNumberX(idx);
		return std::get<0>(pair);
	}

	bool IsNumber(int idx) const
	{
		auto pair = ToNumberX(idx);
		return std::get<1>(pair);
	}

	std::tuple<Int64, bool> ToIntegerX(int idx) const
	{
		LuaValue value = stack->Get(idx);
		return ConvertToInteger(value);
	}

	Int64 ToInteger(int idx) const
	{
		auto pair = ToIntegerX(idx);
		return std::get<0>(pair);
	}

	std::tuple<String, bool> ToStringX(int idx)
	{
		LuaValue val = stack->Get(idx);
		switch(val.tag)
		{
			case LUA_TSTRING: return std::make_tuple(val.str, true);
			case LUA_TNUMBER:
			{
				std::tuple<String, bool> ret;
				if(val.isfloat)
					ret = std::make_tuple(std::to_string(val.number), true);
				else
					ret = std::make_tuple(std::to_string(val.integer), true);
				stack->Set(idx, LuaValue(std::get<0>(ret)));
				return ret;
			}
			default: return std::make_tuple("", false);
		}
	}

	String ToString(int idx)
	{
		auto pair = ToStringX(idx);
		return std::get<0>(pair);
	}

	void Arith(ArithOp op)
	{
		LuaValue b = *stack->Pop();
		LuaValue a;
		if(op != LUA_OPUNM && op != LUA_OPBNOT)
			a = *stack->Pop();
		else
			a = b;
		Operator operaotr = operators[op];
		LuaValue res = _Arith(a, b, operaotr);
		if(res != LuaValue::Nil)
		{
			stack->Push(res);
			return;
		}
		// Call method only if value can not be converted into number
		const String& mm = operaotr.metamethod;
		auto metaRes = CallMetamethod(a, b, mm, this);
		if(std::get<1>(metaRes))
		{
			stack->Push(std::get<0>(metaRes));
			return;
		}
		panic("arithmetic error!");
	}

	bool Compare(int idx1, int idx2, CompareOp op)
	{
		LuaValue a = stack->Get(idx1);
		LuaValue b = stack->Get(idx2);
		switch (op)
		{
			case LUA_OPEQ: return _eq(a, b, this, false);
			case LUA_OPLT: return _lt(a, b, this, false);
			case LUA_OPLE: return _le(a, b, this, false);
			default: panic("invalid compare op!"); return false;
		}
	}

	bool RawEqual(int idx1, int idx2)
	{
		LuaValue a = stack->Get(idx1);
		LuaValue b = stack->Get(idx2);
		return _eq(a, b, this, true);
	}

	void Len(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsString())
			stack->Push(LuaValue((Int64)val.str.length()));
		else
		{
			auto metaRes = CallMetamethod(val, val, "__len", this);
			if(std::get<1>(metaRes))
			{
				stack->Push(std::get<0>(metaRes));
			}
			else if(val.IsTable())
			{
				stack->Push(LuaValue((Int64)val.table->Len()));
			}
			else
			{
				panic("length error!");
			}
		}
	}

	void RawLen(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsString())
			stack->Push(LuaValue((Int64)val.str.length()));
		else
			panic("length error!");
	}

	void Concat(int n)
	{
		if(n == 0)
		{
			stack->Push(LuaValue(""));
		}
		else if(n > 1)
		{
			for(int i = 1; i < n; ++i)
			{
				if(IsString(-1) && IsString(-2))
				{
					String s1 = ToString(-2);
					String s2 = ToString(-1);
					stack->Pop();
					stack->Pop();
					stack->Push(LuaValue(s1 + s2));
					continue;
				}

				LuaValue b = *stack->Pop();
				LuaValue a = *stack->Pop();
				auto metaRes = CallMetamethod(a, b, "__concat", this);
				if(std::get<1>(metaRes))
				{
					stack->Push(std::get<0>(metaRes));
					continue;
				}

				panic("concatenation error!");
			}
		}
		// n == 1, do nothing
	}

	/*
	interfaces for table
	*/
	void CreateTable(int nArr, int nRec)
	{
		LuaTablePtr t = NewLuaTable(nArr, nRec);
		stack->Push(LuaValue(t));
	}

	void NewTable()
	{
		CreateTable(0, 0);
	}

	LuaType _GetTable(const LuaValue& t, const LuaValue& k, bool raw)
	{
		if(t.IsTable())
		{
			LuaValue v = *t.table->Get(k);
			if(raw || v != LuaValue::Nil || !t.table->HasMetafield("__index"))
			{
				stack->Push(v);
				return v.tag;
			}
		}

		if(!raw)
		{
			LuaValue mf = GetMetafield(t, "__index", this);
			if(mf != LuaValue::Nil)
			{
				switch (mf.tag)
				{
					case LUA_TTABLE:
					{
						return _GetTable(mf, k, false);
					}
					case LUA_TFUNCTION:
					{
						stack->Push(mf);
						stack->Push(t);
						stack->Push(k);
						Call(2, 1);
						LuaValue v = stack->Get(-1);
						return v.tag;
					}
					default:
						break;
				}
			}
		}

		panic("index error!");
		return LUA_TNONE;
	}

	LuaType GetTable(int idx)
	{
		LuaValue t = stack->Get(idx);
		LuaValue k = *stack->Pop();
		return _GetTable(t, k, false);
	}

	LuaType GetField(int idx, const String& k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k), false);
	}

	LuaType RawGetField(int idx, const String& k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k), true);
	}

	LuaType GetI(int idx, Int64 k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k), false);
	}

	LuaType RawGetI(int idx, Int64 k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k), true);
	}

	void _SetTable(const LuaValue& t, const LuaValue& k, const LuaValue& v, bool raw)
	{
		if(t.IsTable())
		{
			LuaTablePtr tbl = t.table;
			if(raw || *tbl->Get(k) != LuaValue::Nil || !tbl->HasMetafield("__newindex"))
			{
				t.table->Put(k, v);
				return;
			}
		}

		if(!raw)
		{
			LuaValue mf = GetMetafield(t, "__newindex", this);
			if(mf != LuaValue::Nil)
			{
				switch (mf.tag)
				{
					case LUA_TTABLE:
					{
						return _SetTable(mf, k, v, false);
					}
					case LUA_TFUNCTION:
					{
						stack->Push(mf);
						stack->Push(t);
						stack->Push(k);
						stack->Push(v);
						Call(3, 0);
						return;
					}
					default:
						break;
				}
			}
		}

		panic("index error!");
	}

	void SetTable(int idx)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = *stack->Pop();
		LuaValue k = *stack->Pop();
		_SetTable(t, k, v, false);
	}

	void SetField(int idx, const String& k)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = *stack->Pop();
		_SetTable(t, LuaValue(k), LuaValue(v), false);
	}

	void SetI(int idx, Int64 i)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = *stack->Pop();
		_SetTable(t, LuaValue(i), LuaValue(v), false);
	}

	void RawSetI(int idx, Int64 i)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = *stack->Pop();
		_SetTable(t, LuaValue(i), LuaValue(v), true);
	}

	void PushLuaStack(LuaStackPtr s)
	{
		s->prev = stack;
		stack = s;
	}

	void PopLuaStack()
	{
		LuaStackPtr s = stack;
		stack = stack->prev;
		s->prev = nullptr;
	}

	bool IsBinaryChunk(const ByteArray& chunk)
	{
		if(chunk.size() >= 4)
		{
			Byte signature[4];
			static_assert(sizeof(signature) == sizeof(UInt32), "size check");
			memcpy(signature, chunk.data(), sizeof(signature));
			return memcmp(signature, LUA_SIGNATURE, sizeof(signature)) == 0;
		}
		return false;
	}

	int Load(const ByteArray& chunk, const String& chunkName, const String& mode)
	{
		PrototypePtr proto = nullptr;
		if(IsBinaryChunk(chunk))
		{
			proto = Undump(chunk);
		}
		else
		{
			String strChunk;
			strChunk.reserve(chunk.size());
			for(Byte ch : chunk)
			{
				strChunk += ch;
			}
			proto = Compile(strChunk, chunkName);
		}
		ClosurePtr closure = NewLuaClosure(proto);
		stack->Push(LuaValue(closure));
		if(proto->Upvalues.size() > 0)
		{
			LuaValuePtr env = registry->Get(LuaValue(LUA_RIDX_GLOBALS));
			closure->upvals[0] = UpValue(env);
		}
		return LUA_OK;
	}

	void RunLuaClosure()
	{
		DEBUG_PRINT("Run Lua Closure");
		while(true)
		{
			Instruction inst = Instruction(Fetch());
#if DEBUG_PRINT_ENABLE
			DEBUG_PRINT("%s", inst.OpName().c_str());
			Prototype::PrintOperands(inst);
			puts("");
			// if(inst.Opcode() == OP_VARARG)
			// {
			// 	printf("OP_VARARG");
			// }
#endif
			inst.Execute(this);
#if DEBUG_PRINT_ENABLE
			puts("----------Print Stack Begin----------");
			PrintStack(*this);
			puts("----------Print Stack Finish----------");
#endif
			if(inst.Opcode() == OP_RETURN)
				break;
		}
	}

	void CallLuaClosure(int nArgs, int nResults, ClosurePtr c)
	{
		int nRegs = (int)c->proto->MaxStackSize;
		int nParams = (int)c->proto->NumParams;
		bool isVararg = c->proto->IsVararg == 1;

		LuaStackPtr newStack = NewLuaStack(nRegs + LUA_MINSTACK, stack->state);
		// Assign the function
		newStack->closure = c;

		// a = func(1,2,3,...)
		// a(1,2,3,4,5) nArgs = 5, nParams = 3
		// nArgs(which is all arguments) contains nParams(which is non varargs arguments)
		LuaValueArray funcAndArgs = stack->PopN(nArgs + 1);
		// Push the non varargs arguments
		newStack->PushN(Slice(funcAndArgs, 1, funcAndArgs.size()), nParams);
		panic_cond(nRegs >= nParams, "nRegs is less than nParams");
		newStack->top = nRegs;
		if(nArgs > nParams && isVararg)
		{
			// Assign the varargs arguments
			newStack->varargs = Slice(funcAndArgs, nParams + 1, funcAndArgs.size());
		}

		PushLuaStack(newStack);
		RunLuaClosure();
		PopLuaStack();

		if(nResults != 0)
		{
			// Pop the return values from the newStack
			LuaValueArray results = newStack->PopN(newStack->top - nRegs);
			stack->Check((int)results.size());
			// nResults not "(int)results.size()" for some ticky usage
			stack->PushN(results, nResults);
		}
	}

	void CallCClosure(int nArgs, int nResults, ClosurePtr c)
	{
		LuaStackPtr newStack = NewLuaStack(nArgs + LUA_MINSTACK, stack->state);
		newStack->closure = c;

		LuaValueArray args = stack->PopN(nArgs);
		newStack->PushN(args, nArgs);
		// Throw away the C closure
		stack->Pop();

		PushLuaStack(newStack);
		int r = c->cFunc(this);
		PopLuaStack();

		if(nResults != 0)
		{
			LuaValueArray results = newStack->PopN(r);
			newStack->Check((int)results.size());
			stack->PushN(results, nResults);
		}
	}

	void Call(int nArgs, int nResults)
	{
		// closure func
		LuaValue val = stack->Get(-(nArgs + 1));

		if(!val.IsClosure())
		{
			LuaValue mf = GetMetafield(val, "__call", this);
			if(mf != LuaValue::Nil)
			{
				if(mf.IsClosure())
				{
					stack->Push(mf);
					Insert(-(nArgs + 2));
					nArgs += 1;
					val = mf;
				}
			}
		}

		if(val.IsClosure())
		{
			ClosurePtr c = val.closure;
			if(c->proto)
			{
				DEBUG_PRINT("call %s<%d,%d>", c->proto->Source.c_str(),
					c->proto->LineDefined,
			 		c->proto->LastLineDefined);
			}
			else
			{
				DEBUG_PRINT("call c function");
			}

			if(c->proto != nullptr)
				CallLuaClosure(nArgs, nResults, c);
			else
				CallCClosure(nArgs, nResults, c);
		}
		else
		{
			panic("not a function");
		}
	}

	void PushCFunction(CFunction c, int n)
	{
		ClosurePtr closure = NewCClosure(c, n);
		for(int i = n; i > 0; --i)
		{
			LuaValuePtr val = stack->Pop();
			closure->upvals[i - 1] = UpValue(val);
		}
		stack->Push(LuaValue(closure));
	}

	bool IsCFunction(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsClosure())
		{
			return val.closure->cFunc != nullptr;
		}
		return false;
	}

	CFunction ToCFunction(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsClosure())
		{
			return val.closure->cFunc;
		}
		return nullptr;
	}

	void PushGlobalTable()
	{
		LuaValue global = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		stack->Push(global);
	}

	LuaType GetGlobal(const String& name)
	{
		LuaValue t = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		return _GetTable(t, LuaValue(name), true);
	}

	void SetGlobal(const String& name)
	{
		LuaValue t = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		LuaValue v = *stack->Pop();
		_SetTable(t, LuaValue(name), v, true);
	}

	void Register(const String& name, CFunction f)
	{
		PushCFunction(f, 0);
		SetGlobal(name);
	}

	bool GetMetatable(int idx)
	{
		LuaValue val = stack->Get(idx);
		LuaTablePtr mt = ::GetMetatable(val, this);
		if(mt)
		{
			stack->Push(LuaValue(mt));
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetMetatable(int idx)
	{
		LuaValue val = stack->Get(idx);
		LuaValue mtVal = *stack->Pop();
		if(mtVal == LuaValue::Nil)
		{
			::SetMetatable(val, LuaTable::NilPtr, this);
		}
		else if(mtVal.IsTable())
		{
			::SetMetatable(val, mtVal.table, this);
		}
		else
		{
			panic("table expected!"); // todo
		}
	}

	bool Next(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsTable())
		{
			LuaValue key = *stack->Pop();
			LuaTablePtr tbl = val.table;
			LuaValue nextKey = tbl->NextKey(key);
			if(nextKey != LuaValue::Nil)
			{
				stack->Push(nextKey);
				stack->Push(*tbl->Get(nextKey));
				return true;
			}
			return false;
		}
		panic("table expected!");
		return false;
	}

	int Error()
	{
		LuaValue err = *stack->Pop();
		panic(err.str.c_str());
		return LUA_ERRRUN;
	}

	int PCall(int nArgs, int nResults, int msgh)
	{
		int status = LUA_ERRRUN;
		LuaStackPtr caller = stack;

		try
		{
			Call(nArgs, nResults);
			status = LUA_OK;
		}
		catch(const String& msg)
		{
			while(stack != caller)
			{
				PopLuaStack();
			}
			stack->Push(LuaValue(msg));
		}
		catch(...)
		{
			printf("panic\n");
			exit(0);
		}

		return status;
	}

	/*
	interfaces for luavm
	*/
	int PC() const { return stack->pc; }

	void AddPC(int n) { stack->pc += n; }

	UInt32 Fetch() { return stack->closure->proto->Code[stack->pc++]; }

	void GetConst(int idx)
	{
		Constant c = stack->closure->proto->Constants[idx];
		switch (c.tag)
		{
			case TAG_NIL: stack->Push(LuaValue::Nil); break;
			case TAG_BOOLEAN: stack->Push(LuaValue(c.boolean)); break;
			case TAG_NUMBER: stack->Push(LuaValue(c.luaNum)); break;
			case TAG_INTEGER: stack->Push(LuaValue(c.luaInteger)); break;
			case TAG_SHORT_STR:
			case TAG_LONG_STR: stack->Push(LuaValue(c.str)); break;
		}
	}

	void GetRK(int rk)
	{
		if(rk > 0xFF)
			GetConst(rk & 0xFF);
		else
			PushValue(rk + 1);
	}

	int RegisterCount() const { return stack->closure->proto->MaxStackSize;	}

	void LoadVararg(int n)
	{
		if(n < 0)
			n = stack->varargs.size();
		stack->Check(n);
		stack->PushN(stack->varargs, n);
	}

	void LoadProto(int idx)
	{
		PrototypePtr subProto = stack->closure->proto->Protos[idx];
		ClosurePtr closure = NewLuaClosure(subProto);
		stack->Push(LuaValue(closure));

		for(size_t i = 0; i < subProto->Upvalues.size(); ++i)
		{
			const Upvalue& uvInfo = subProto->Upvalues[i];
			int uvIdx = uvInfo.Idx;

			if(uvInfo.Instack == 1)
			{
				auto it = stack->openuvs.find(uvIdx);
				if(it == stack->openuvs.end())
				{
					closure->upvals[i] = UpValue(stack->slots[uvIdx]);
					stack->openuvs[uvIdx] = closure->upvals[i];
				}
				else
				{
					closure->upvals[i] = it->second;
				}
			}
			else
			{
				closure->upvals[i] = stack->closure->upvals[uvIdx];
			}
		}
	}

	void CloseUpvalues(int a)
	{
		for(auto it = stack->openuvs.begin(); it != stack->openuvs.end();)
		{
			int i = it->first;
			if(i >= a - 1)
			{
				it = stack->openuvs.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
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