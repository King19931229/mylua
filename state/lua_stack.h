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
		assert(idx != 0);
		if(idx >= 0)
			return idx;
		else
			return idx + (int)top + 1;		
	}

	bool IsValid(int idx) const
	{
		size_t absIdx = AbsIndex(idx);
		return absIdx > 0 && absIdx <= top;
	}

	LuaValue Get(int idx)
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
	void Reverse(size_t from, size_t to)
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