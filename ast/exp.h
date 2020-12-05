#pragma once
#include "struct.h"

struct NilExp : public Exp
{
	int Line;
};

struct TrueExp : public Exp
{
	int Line;
};

struct FalseExp : public Exp
{
	int Line;
};

struct VarargExp : public Exp
{
	int Line;
};

struct IntegerExp : public Exp
{
	int Line;
	Int64 Val;
};

struct FloatExp : public Exp
{
	int Line;
	Float64 Val;
};

struct StringExp : public Exp
{
	int Line;
	String Val;
};

struct NameExp : public Exp
{
	int Line;
	String Name;
};

struct UnopExp : public Exp
{
	int Line;
	int Op;
	ExpPtr Exp;
};

struct BinopExp : public Exp
{
	int Line;
	int Op;
	ExpPtr Exp1;
	ExpPtr Exp2;
};

struct ConcatExp : public Exp
{
	int Line;
	std::vector<ExpPtr> Exps;
};

// tableconstructor ::= '{' [fieldlist] '}'
// fieldlist ::= field {fieldsep field} [fieldsep]
// field ::= '[' exp ']' '=' exp | Name '=' exp | exp
// fieldsep ::= ',' | ';'
struct TableConstructorExp : public Exp
{
	int Line;
	int LastLine;
	std::vector<ExpPtr> KeyExps;
	std::vector<ExpPtr> ValExps;
};

// functiondef ::= function funcbody
// funcbody ::= '(' [parlist] ')' block end
// parlist ::= namelist [',' '...'] | '...'
// namelist ::= Name {',' Name}
struct FuncDefExp : public Exp
{
	int Line;
	int LastLine;
	std::vector<String> ParList;
	bool IsVararg;
	BlockPtr Block;
};

struct ParensExp : public Exp
{
	ExpPtr Exp;
};

struct TableAccessExp : public Exp
{
	int LastLine;
	ExpPtr PrefixExp;
	ExpPtr KeyExp;
};

// functioncall ::= prefixexp [':' Name] args
// args ::= '(' [explist] ')' | tableconstructor | LiteralString
struct FuncCallExp : public Exp
{
	int Line;
	int LastLine;
	ExpPtr PrefixExp;
	// StringExp
	ExpPtr NameExp;
	std::vector<ExpPtr> Args;
};