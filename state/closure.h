#pragma once
#include "binchunk/binary_chunk.h"

typedef int(*CFunction)(LuaState* state);

struct Closure
{
	PrototypePtr proto;
	CFunction cFunc;

	explicit Closure(PrototypePtr p)
	{
		proto = p;
		cFunc = nullptr;
	}

	explicit Closure(CFunction c)
	{
		proto = nullptr;
		cFunc = c;
	}
};

inline ClosurePtr NewLuaClosure(PrototypePtr proto)
{
	return ClosurePtr(new Closure(proto));
}

inline ClosurePtr NewCClosure(CFunction c)
{
	return ClosurePtr(new Closure(c));
}