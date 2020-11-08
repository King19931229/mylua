#pragma once
#include "lua_stack.h"

enum
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

enum
{
	LUA_OPEQ,
	LUA_OPLT,
	LUA_OPLE,
};

struct LuaState
{
	LuaStack stack;

	int GetTop() const
	{
		return (int)stack.top;
	}

	int AbsIndex(int idx) const
	{
		return stack.AbsIndex(idx);
	}

	bool CheckStack(int n)
	{
		stack.Check(n);
		return true;
	}

	void Pop(int n)
	{
		for(int i = 0; i < n; ++i)
			stack.Pop();
	}

	void Copy(int fromIdx, int toIdx)
	{
		LuaValue val = stack.Get(fromIdx);
		stack.Set(toIdx, val);
	}

	void PushValue(int idx)
	{
		LuaValue val = stack.Get(idx);
		stack.Push(val);
	}

	void Replace(int idx)
	{
		LuaValue val = stack.Pop();
		stack.Set(idx, val);
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
		size_t t = stack.top - 1;
		size_t p = idx - 1;
		size_t m = (n >= 0) ? (t - n) : (p - n - 1);
		stack._Reverse(p, m);
		stack._Reverse(m + 1, t);
		stack._Reverse(p, t);
	}

	void SetTop(int idx)
	{
		size_t newTop = stack.AbsIndex(idx);
		if(newTop < 0)
		{
			panic("stack underflow");
		}
		int n = (int)stack.top - (int)newTop;
		if(n > 0)
		{
			for(int i = 0; i < n; ++i)
				stack.Pop();
		}
		else if(n < 0)
		{
			for(int i = 0; i > n; --i)
				stack.Push(LuaValue::Nil);
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

	void PushNil() { stack.Push(LuaValue::Nil); }
	void PushBoolean(bool b) { stack.Push(LuaValue(b)); }
	void PushInteger(Int64 n) { stack.Push(LuaValue(n)); }
	void PushNumber(Float64 n) { stack.Push(LuaValue(n)); }
	void PushString(const String& str){ stack.Push(LuaValue(str)); }

	static String TypeName(LuaType tp)
	{
		switch (tp)
		{
			case LUA_TNONE: return "no value";
			case LUA_TNIL: return "nil";
			case LUA_TBOOLEAN: return "boolean";
			case LUA_TNUMBER: return "number";
			case LUA_TSTRING: return "string";
			case LUA_TTABLE: return "table";
			case LUA_TFUNCTION: return "function";	
			case LUA_TTHREAD: return "thread";
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			default: return "userdata";
		}
	}

	LuaType Type(int idx) const
	{
		if(stack.IsValid(idx))
		{
			LuaValue value = stack.Get(idx);
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
		LuaValue value = stack.Get(idx);
		return ConvertToBoolean(value);
	}

	std::tuple<Float64, bool> ToNumberX(int idx) const
	{
		LuaValue value = stack.Get(idx);
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
		LuaValue value = stack.Get(idx);
		return ConvertToInteger(value);
	}

	Int64 ToInteger(int idx) const
	{
		auto pair = ToIntegerX(idx);
		return std::get<0>(pair);
	}

	std::tuple<String, bool> ToStringX(int idx)
	{
		LuaValue val = stack.Get(idx);
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
				stack.Set(idx, LuaValue(std::get<0>(ret)));
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
};

inline LuaState NewLuaState()
{
	return LuaState{NewLuaStack(20)};
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
			default: printf("[%s]",  state.TypeName(t).c_str()); break;
		}
	}
	puts("");
}