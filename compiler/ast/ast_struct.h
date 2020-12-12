#pragma once
#include "public.h"

struct Stat;
using StatPtr = std::shared_ptr<Stat>;

struct Stat : public RTTI
{
	template<typename T>
	static StatPtr New()
	{
		StatPtr ret = StatPtr(new T());
		return ret;
	}
};
using StatArray = std::vector<StatPtr>;

struct Exp;
using ExpPtr = std::shared_ptr<Exp>;

struct Exp : public RTTI
{
	template<typename T>
	static ExpPtr New()
	{
		ExpPtr ret = ExpPtr(new T());
		return ret;
	}
};
using ExpArray = std::vector<ExpPtr>;

struct Block;
using BlockPtr = std::shared_ptr<Block>;
using BlockArray = std::vector<BlockPtr>;