#pragma once
#include "lua_value.h"
#include "lua_table.h"
#include "closure.h"
#include "public.h"
#include <vector>
#include <assert.h>

struct LuaStack
{
	std::vector<LuaValuePtr> slots;
	std::unordered_map<int, UpValue> openuvs;
	LuaValueArray varargs;
	LuaStackPtr prev;
	ClosurePtr closure;
	LuaState* state;
	int top;
	int pc;

	LuaStack();
	void Check(int n);
	void Push(const LuaValue& value);
	LuaValuePtr Pop();
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
		ret->slots[i] = LuaValue::NilPtr;
	ret->top = 0;
	ret->state = state;
	return ret;
}