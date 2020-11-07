#pragma once
#include "lua_stack.h"

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

	void Push(int idx)
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
		stack.Reverse(p, m);
		stack.Reverse(m + 1, t);
		stack.Reverse(p, t);
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
	void PushInteger(UInt64 n) { stack.Push(LuaValue(n)); }
	void PushNumber(double n) { stack.Push(LuaValue(n)); }
	void PushNil(const String& str){ stack.Push(LuaValue(str)); }
};

inline LuaState New()
{
	return LuaState{NewLuaStack(20)};
}