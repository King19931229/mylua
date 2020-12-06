#pragma once
#include "ast_struct.h"

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
struct GotoStat : public Stat
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
	ExpArray Exps;
	BlockArray Blocks;
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
	StringArray NameList;
	ExpArray ExpList;
	BlockPtr Block;
};

// local namelist [= explist]
struct LocalVarDeclList : public Stat
{
	int Line;
	StringArray NameList;
	ExpArray ExpList;
};

// varlist = explist
struct AssignStat : public Stat
{
	int Line;
	StringArray VarList;
	ExpArray ExpList;
};

// -> 0.local function f (params) body end
// -> 1.local f; f = function (params) body end
struct LocalFuncDefStat : public Stat
{
	String Name;
	// FuncDefExp
	ExpPtr Exp;
};