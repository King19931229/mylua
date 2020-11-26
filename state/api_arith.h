#pragma once
#include "api/consts.h"
#include "number/math.h"
#include "number/parser.h"
#include "state/lua_value.h"
#include <functional>

typedef Int64 (*IntegerFunc)(Int64, Int64);
typedef Float64 (*FloatFunc)(Float64, Float64);

struct Operator
{
	String metamethod;
	IntegerFunc IntegerFunc;
	FloatFunc FloatFunc;
};

extern const Operator operators[14];

inline LuaValue _Arith(const LuaValue& a, const LuaValue& b, Operator op)
{
	// bitwise
	if(op.FloatFunc == nullptr)
	{
		auto aRes = ConvertToInteger(a);
		if(std::get<1>(aRes))
		{
			auto bRes = ConvertToInteger(b);
			if(std::get<1>(bRes))
				return LuaValue(op.IntegerFunc(std::get<0>(aRes), std::get<0>(bRes)));
		}
	}
	// arith
	else
	{
		if(op.IntegerFunc != nullptr)
		{
			// both is int
			if(a.IsInt64() && b.IsInt64())
				return LuaValue(op.IntegerFunc(a.integer, b.integer));
		}
		auto aRes = ConvertToFloat(a);
		if(std::get<1>(aRes))
		{
			auto bRes = ConvertToFloat(b);
			if(std::get<1>(bRes))
				return LuaValue(op.FloatFunc(std::get<0>(aRes), std::get<0>(bRes)));
		}
	}
	return LuaValue::Nil;
}