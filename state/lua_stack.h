#pragma once
#include "lua_value.h"
#include "type.h"
#include <vector>
#include <assert.h>

struct LuaStack
{
	std::vector<LuaValue> slots;
	size_t top;

	void Check(size_t n)
	{
		size_t free = slots.size() - top;
		for(size_t i = free; i < n; ++i)
		{
			slots.push_back(LuaValue::Nil);
		}
	}

	void Push(const LuaValue& value)
	{
		if(top == slots.size())
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

	size_t AbsIndex(int idx) const
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
		size_t absIdx = AbsIndex(idx);
		return absIdx > 0 && absIdx <= top;
	}

	// idx here is absoulte index [1,n]
	LuaValue Get(int idx) const
	{
		size_t absIdx = AbsIndex(idx);
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
		size_t absIdx = AbsIndex(idx);
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

inline LuaStack NewLuaStack(size_t size)
{
	LuaStack ret;
	ret.slots.resize(size);
	for(size_t i = 0; i < size; ++i)
		ret.slots[i] = LuaValue::Nil;
	ret.top = 0;
	return ret;
}