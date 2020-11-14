#pragma once
#include "public.h"
#include <math.h>

inline Int64 IFloorDiv(Int64 a, Int64 b)
{
	if((a > 0 && b > 0) || (a < 0 && b < 0) || (a % b == 0))
		return a / b;
	else
		return a / b - 1;
}

inline Float64 FFloorDiv(Float64 a, Float64 b)
{
	return floor(a / b);
}

inline Int64 IMod(Int64 a, Int64 b)
{
	return a - IFloorDiv(a, b) * b;
}

inline Float64 FMod(Float64 a, Float64 b)
{
	return a - FFloorDiv(a, b) * b;
}

inline Int64 ShiftLeft(Int64 a, Int64 n)
{
	extern Int64 ShiftRight(Int64, Int64);
	if(n >= 0)
		return a << n;
	else
		return ShiftRight(a, -n);
}

inline Int64 ShiftRight(Int64 a, Int64 n)
{
	extern Int64 ShiftLeft(Int64, Int64);
	if(n >= 0)
		return (Int64)((UInt64)a >> n);
	else
		return ShiftLeft(a, -n);
}