#pragma once
#include "ast_struct.h"

struct Block
{
	int LastLine;
	StatArray Stats;
	ExpArray RetExps;
};