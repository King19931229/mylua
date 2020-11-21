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
	LuaState* state;
	int top;
	int pc;

	LuaStack();
	void Check(int n);
	void Push(const LuaValue& value);
	LuaValue Pop();
	void PushN(const LuaValueArray& vals, int n);
	LuaValueArray PopN(int n);
	int AbsIndex(int idx) const;
	// idx here is absoulte index [1,n]
	bool IsValid(int idx) const;
	// idx here is absoulte index [1,n]
	LuaValue Get(int idx) const;
	// idx here is absoulte index [1,n]
	void Set(int idx, const LuaValue& value);
	// from and to is internal index
	void _Reverse(size_t from, size_t to);
};

inline LuaStackPtr NewLuaStack(int size, LuaState* state)
{
	LuaStackPtr ret = LuaStackPtr(new LuaStack());
	ret->slots.resize(size);
	for(int i = 0; i < size; ++i)
		ret->slots[i] = LuaValue::Nil;
	ret->top = 0;
	ret->state = state;
	return ret;
}