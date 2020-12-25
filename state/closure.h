#pragma once
#include "binchunk/binary_chunk.h"

typedef int(*CFunction)(LuaState* state);

struct FuncReg
{
	const char* name;
	CFunction func;

	FuncReg()
	{
		name = nullptr;
		func = nullptr;
	}

	FuncReg(const char* _name, CFunction _func)
	{
		name = _name;
		func = _func;
	}
};

struct UpValue
{
	LuaValuePtr val;

	UpValue()
	{
		val = nullptr;
	}

	explicit UpValue(LuaValuePtr _val)
	{
		val = _val;
	}
};

struct Closure
{
	PrototypePtr proto;
	CFunction cFunc;
	std::vector<UpValue> upvals;

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
	ClosurePtr closure = ClosurePtr(new Closure(proto));
	closure->upvals.resize(proto->Upvalues.size());
	return closure;
}

inline ClosurePtr NewCClosure(CFunction c, int nUpVals)
{
	ClosurePtr closure = ClosurePtr(new Closure(c));
	if(nUpVals > 0)
		closure->upvals.resize(nUpVals);
	return closure;
}