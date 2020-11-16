#pragma once
#include "binchunk/binary_chunk.h"

struct Closure
{
	PrototypePtr proto;
	Closure(PrototypePtr p)
	{
		proto = p;
	}
};

using ClosurePtr = std::shared_ptr<Closure>;

inline ClosurePtr NewLuaClosure(PrototypePtr proto)
{
	return ClosurePtr(new Closure(proto));
}