#pragma once
#include "struct.h"

struct Block
{
	int LastLine;
	std::vector<StatPtr> Stats;
	std::vector<ExpPtr> RetExps;
};