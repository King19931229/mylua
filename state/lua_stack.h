#pragma once
#include "lua_value.h"
#include "lua_table.h"
#include "closure.h"
#include "public.h"
#include <vector>
#include <assert.h>

struct LuaStack;
using LuaStackPtr = std::shared_ptr<LuaStack>;

struct LuaStack
{
	std::vector<LuaValue> slots;
	std::vector<LuaValue> varargs;
	LuaStackPtr prev;
	ClosurePtr closure;
	int top;
	int pc;

	LuaStack()
	{
		prev = nullptr;
		closure = nullptr;
		top = pc = 0;
	}

	void Check(int n)
	{
		int free = (int)slots.size() - top;
		for(int i = free; i < n; ++i)
		{
			slots.push_back(LuaValue::Nil);
		}
	}

	void Push(const LuaValue& value)
	{
		if(top == (int)slots.size())
		{
			panic("stack overflow!");
		}
		else
		{
			slots[top++] = value;
		}
	}

	LuaValue Pop()
	{
		if(top < 1)
		{
			panic("stack underflow!");
			return LuaValue::Nil;
		}
		else
		{
			--top;
			LuaValue ret = slots[top];
			slots[top] = LuaValue::Nil;
			return ret;	
		}
	}

	void PushN(const LuaValueArray& vals, int n)
	{
		for(int i = 0; i < n; ++i)
		{
			if(i < (int)vals.size())
				Push(vals[i]);
			else
				Push(LuaValue::Nil);
		}
	}

	LuaValueArray PopN(int n)
	{
		LuaValueArray vals;
		vals.resize(n);
		for(int i = 0; i < n; ++i)
			vals[i] = Pop();
		return vals;
	}

	int AbsIndex(int idx) const
	{
		panic_cond(idx != 0, "index out of bound");
		if(idx >= 0)
			return idx;
		else
		{
			panic_cond(top + idx >= 0, "index out of bound");
			return idx + (int)top + 1;
		}
	}

	// idx here is absoulte index [1,n]
	bool IsValid(int idx) const
	{
		int absIdx = AbsIndex(idx);
		return absIdx > 0 && absIdx <= top;
	}

	// idx here is absoulte index [1,n]
	LuaValue Get(int idx) const
	{
		int absIdx = AbsIndex(idx);
		if(absIdx > 0 && absIdx <= top)
		{
			return slots[absIdx - 1];
		}
		else
		{
			return LuaValue::Nil;
		}
	}

	// idx here is absoulte index [1,n]
	void Set(int idx, const LuaValue& value)
	{
		int absIdx = AbsIndex(idx);
		if(absIdx > 0 && absIdx <= top)
		{
			slots[absIdx - 1] = value;
		}
		else
		{
			panic("invalid index!");
		}
	}

	// from and to is internal index
	void _Reverse(size_t from, size_t to)
	{
		while(from < to)
		{
			LuaValue temp = slots[from];
			slots[from] = slots[to];
			slots[to] = temp;
		}
	}
};

inline LuaStackPtr NewLuaStack(size_t size)
{
	LuaStackPtr ret = LuaStackPtr(new LuaStack());
	ret->slots.resize(size);
	for(size_t i = 0; i < size; ++i)
		ret->slots[i] = LuaValue::Nil;
	ret->top = 0;
	return ret;
}