#pragma once
#include "public.h"

struct Stat
{	
};
using StatPtr = std::shared_ptr<Stat>;

struct Exp
{
};
using ExpPtr = std::shared_ptr<Exp>;

struct Block;
using BlockPtr = std::shared_ptr<Block>;

struct FunctionCallExp;