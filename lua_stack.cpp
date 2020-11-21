#include "state/lua_value.h"
#include "state/lua_table.h"
#include "state/lua_stack.h"
#include "state/lua_state.h"

const LuaValue LuaValue::NoValue(LUA_TNONE);
const LuaValue LuaValue::Nil(LUA_TNIL);

size_t LuaValue::Hash() const
{
	size_t hash = 0;
	HashCombine(hash, tag);
	HashCombine(hash, integer);
	HashCombine(hash, isfloat);
	HashCombine(hash, _BKDR(str.c_str(), str.length()));
	HashCombine(hash, table ? (size_t)table.get() : 0);
	// Don't be stupid to hash the closure pointer
	if(closure)
	{
		if(closure->proto)
		{
			HashCombine(hash, (size_t)closure->proto.get());
		}
		else if(closure->cFunc)
		{
			HashCombine(hash, (size_t)closure->cFunc);
		}
		else
		{
			HashCombine(hash, 0);
		}
	}
	return hash;
}

LuaStack::LuaStack()
{
	prev = nullptr;
	closure = nullptr;
	state = nullptr;
	top = pc = 0;
}

void LuaStack::Check(int n)
{
	int free = (int)slots.size() - top;
	for(int i = free; i < n; ++i)
	{
		slots.push_back(LuaValue::Nil);
	}
}

void LuaStack::Push(const LuaValue& value)
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

LuaValue LuaStack::Pop()
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

void LuaStack::PushN(const LuaValueArray& vals, int n)
{
	if(n < 0)
		n = (int)vals.size();
	for(int i = 0; i < n; ++i)
	{
		if(i < (int)vals.size())
			Push(vals[i]);
		else
			Push(LuaValue::Nil);
	}
}

LuaValueArray LuaStack::PopN(int n)
{
	LuaValueArray vals;
	vals.resize(n);
	for(int i = 0; i < n; ++i)
		vals[i] = Pop();
	return vals;
}

int LuaStack::AbsIndex(int idx) const
{
	panic_cond(idx != 0, "index out of bound");

	if(idx <= /* or == */ LUA_REGISTRYINDEX)
	{
		return idx;
	}

	if(idx >= 0)
	{
		return idx;
	}
	else
	{
		panic_cond(top + idx >= 0, "index out of bound");
		return idx + (int)top + 1;
	}
}

// idx here is absoulte index [1,n]
bool LuaStack::IsValid(int idx) const
{
	if(idx == /* or <= */ LUA_REGISTRYINDEX)
	{
		return true;
	}

	int absIdx = AbsIndex(idx);
	return absIdx > 0 && absIdx <= top;
}

// from and to is internal index
void LuaStack::_Reverse(size_t from, size_t to)
{
	while(from < to)
	{
		LuaValue temp = slots[from];
		slots[from] = slots[to];
		slots[to] = temp;
	}
}

LuaValue LuaStack::Get(int idx) const
{
	if(idx == LUA_REGISTRYINDEX)
	{
		return LuaValue(state->registry);
	}

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

void LuaStack::Set(int idx, const LuaValue& value)
{
	if(idx == LUA_REGISTRYINDEX)
	{
		if(value.IsTable())
			state->registry = value.table;
		return;
	}

	int absIdx = AbsIndex(idx);

	while(absIdx > top)
		++top;
	panic_cond(top <= (int)slots.size(), "top out of bound");

	if(absIdx > 0 && absIdx <= top)
	{
		slots[absIdx - 1] = value;
	}
	else
	{
		panic("invalid index!");
	}
}