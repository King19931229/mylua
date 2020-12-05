#pragma once
#include "struct.h"

// ;
struct EmptyStat : public Stat
{
};

// break
struct BreakStat : public Stat
{
	int Line;
};

// :: Name
struct LabelStat : public Stat
{
	String Name;
};

// goto Name
struct LabelStat : public Stat
{
	String Name;
};

// do block end
struct DoStat : public Stat
{
	BlockPtr Block;
};

// functioncall
using FunctionCallStat = FunctionCallExp;

// while exp do block end
struct WhileStat : public Stat
{
	ExpPtr Exp;
	BlockPtr Block;
};

// repeat block until exp
struct RepeatStat : public Stat
{
	BlockPtr Block;
	ExpPtr Exp;
};

// -> 0.if exp then block {elseif exp then block} [else block] end
// -> 1.if exp then block {elseif exp then block} [elseif true then block] end
// -> 2.if exp then block {elseif exp then block} end
struct IfStat : public Stat
{
	std::vector<ExpPtr> Exps;
	std::vector<BlockPtr> Blocks;
};

// for Name = exp , exp [, exp] do block end
struct ForNumStat : public Stat
{
	int LineOfFor;
	int LineOfDo;
	String VarName;
	ExpPtr InitExp;
	ExpPtr LimitExp;
	ExpPtr StepExp;
	BlockPtr Block;
};

// for namelist in explist do block end
struct ForInStat : public Stat
{
	int LineOfDo;
	std::vector<String> NameList;
	std::vector<ExpPtr> ExpList;
	BlockPtr Block;
};

// local namelist [= explist]
struct LocalVarDeclList : public Stat
{
	int Line;
	std::vector<String> NameList;
	std::vector<ExpPtr> ExpList;
};

// varlist = explist
struct AssignStat : public Stat
{
	int Line;
	std::vector<String> VarList;
	std::vector<ExpPtr> ExpList;
};

// -> 0.local function f (params) body end
// -> 1.local f; f = function (params) body end
struct LocalFuncDefStat : public Stat
{
	String Name;
	// FuncDefExp
	ExpPtr Exp;
};