#pragma once
#include "lua_stack.h"
#include "api_arith.h"
#include "api_compare.h"
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"

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

struct LuaState;
using LuaStatePtr = std::shared_ptr<LuaState>;

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
		LuaValue val = stack->Pop();
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
		size_t p = idx - 1;
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
		LuaValue b = stack->Pop();
		LuaValue a;
		if(op != LUA_OPUNM && op != LUA_OPBNOT)
			a = stack->Pop();
		else
			a = b;
		Operator operaotr = operators[op];
		LuaValue res = _Arith(a, b, operaotr);
		if(res != LuaValue::Nil)
			stack->Push(res);
		else
			panic("arithmetic error!");
	}

	bool Compare(int idx1, int idx2, CompareOp op) const
	{
		LuaValue a = stack->Get(idx1);
		LuaValue b = stack->Get(idx2);
		switch (op)
		{
			case LUA_OPEQ: return _eq(a, b);
			case LUA_OPLT: return _lt(a, b);
			case LUA_OPLE: return _le(a, b);
			default: panic("invalid compare op!"); return false;
		}
	}

	void Len(int idx)
	{
		LuaValue val = stack->Get(idx);
		if(val.IsString())
			stack->Push(LuaValue((Int64)val.str.length()));
		else if(val.IsTable())
			stack->Push(LuaValue((Int64)val.table->Len()));
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

	LuaType _GetTable(const LuaValue& t, const LuaValue& k)
	{
		if(t.IsTable())
		{
			// void* p = t.table.get();
			// if(p == nullptr)
			// {
			// 	panic("debug");
			// }
			LuaValue v = t.table->Get(k);
			stack->Push(v);
			return v.tag;
		}
		else
		{
			panic("not a table!");
			return LUA_TNONE;
		}
	}

	LuaType GetTable(int idx)
	{
		LuaValue t = stack->Get(idx);
		LuaValue k = stack->Pop();
		return _GetTable(t, k);
	}

	LuaType GetField(int idx, const String& k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k));
	}

	LuaType GetI(int idx, Int64 k)
	{
		LuaValue t = stack->Get(idx);
		return _GetTable(t, LuaValue(k));
	}

	void _SetTable(const LuaValue& t, const LuaValue& k, const LuaValue& v)
	{
		if(t.IsTable())
		{
			// void* p = t.table.get();
			// if(p == nullptr)
			// {
			// 	panic("debug");
			// }
			t.table->Put(k, v);
		}
		else
		{
			panic("not a table!");
		}
	}

	void SetTable(int idx)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = stack->Pop();
		LuaValue k = stack->Pop();
		_SetTable(t, k, v);
	}

	void SetField(int idx, const String& k)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = stack->Pop();
		_SetTable(t, LuaValue(k), LuaValue(v));
	}

	void SetI(int idx, Int64 i)
	{
		LuaValue t = stack->Get(idx);
		LuaValue v = stack->Pop();
		_SetTable(t, LuaValue(i), LuaValue(v));
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

	int Load(const ByteArray& chunk, const String& chunkName, const String& mode)
	{
		PrototypePtr proto = Undump(chunk);
		ClosurePtr closure = NewLuaClosure(proto);
		stack->Push(LuaValue(closure));
		return 0;
	}

	void RunLuaClosure()
	{
		while(true)
		{
			Instruction inst = Instruction(Fetch());
			inst.Execute(this);
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
		if(val.IsClosure())
		{
			ClosurePtr c = val.closure;
			if(c->proto)
			{
				printf("call %s<%d,%d>\n", c->proto->Source.c_str(),
					c->proto->LineDefined,
			 		c->proto->LastLineDefined);
			}
			else
			{
				printf("call cfunction\n");
			}
			
			if(c->proto != nullptr)
				CallLuaClosure(nArgs, nResults, c);
			else
				CallCClosure(nArgs, nResults, c);		
		}
		else
		{
			warning("not function");
		}
	}

	void PushCFunction(CFunction c)
	{
		stack->Push(LuaValue(NewCClosure(c)));
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
		LuaValue global = registry->Get(LuaValue(LUA_RIDX_GLOBALS));

		// void* p1 = registry.get();
		// void* p2 = global.table.get();
		// if(p1 == p2)
		// {
		// 	panic("debug");
		// }

		stack->Push(global);
	}

	LuaType GetGlobal(const String& name)
	{
		LuaValue t = registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		return _GetTable(t, LuaValue(name));
	}

	void SetGlobal(const String& name)
	{
		LuaValue t = registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		LuaValue v = stack->Pop();
		_SetTable(t, LuaValue(name), v);
	}

	void Register(const String& name, CFunction f)
	{
		PushCFunction(f);
		SetGlobal(name);		
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
		PrototypePtr proto = stack->closure->proto->Protos[idx];
		ClosurePtr closure = NewLuaClosure(proto);
		stack->Push(LuaValue(closure));
	}
};

using LuaVM = LuaState;

inline LuaStatePtr NewLuaState()
{
	LuaTablePtr registry = NewLuaTable(0, 0);
	LuaValue global = LuaValue(NewLuaTable(0, 0));
	registry->Put(LuaValue(LUA_RIDX_GLOBALS), global);

	// void* p1 = registry.get();
	// void* p2 = global.table.get();
	// if(p1 == p2)
	// {
	// 	panic("debug");
	// }

	LuaStatePtr ls = LuaStatePtr(new LuaState
		{
			nullptr,
			registry
		}
	);

	ls->PushLuaStack(NewLuaStack(LUA_MINSTACK, ls.get()));

	return ls;
}

inline void PrintStack(LuaState& state)
{
	int top = state.GetTop();
	for(int i = 1; i <= top; ++i)
	{
		LuaType t = state.Type(i);
		switch(t)
		{
			case LUA_TBOOLEAN: printf("[%s]", Format::FromBool(state.ToBoolean(i)).c_str()); break;
			case LUA_TNUMBER: printf("[%s]", Format::FromFloat64(state.ToNumber(i)).c_str()); break;
			case LUA_TSTRING: printf("[%s]", state.ToString(i).c_str()); break;
			default: printf("[%s]", TypeName(t).c_str()); break;
		}
	}
	puts("");
}