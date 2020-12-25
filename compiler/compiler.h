#pragma once
#include "compiler/codegen/func_info.h"

inline PrototypePtr Compile(const String& chunk, const String& chunkName)
{
	BlockPtr ast = Parse(chunk, chunkName);
	return GenProto(ast);
}