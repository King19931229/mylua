#pragma once
#include "compiler/ast/block.h"
#include "compiler/parser/parser.h"
#include "compiler/lexer/lexer.h"
#include "state/lua_value.h"

struct LocVarInfo;
using LocVarInfoPtr = std::shared_ptr<LocVarInfo>;

struct LocVarInfo
{
	LocVarInfoPtr prev;
	String name;
	int scopeLv;
	int slot;
	bool captured;

	LocVarInfo()
	{
		scopeLv = 0;
		slot = 0;
		captured = false;
	}
};

struct UpvalInfo;
using UpvalInfoPtr = std::shared_ptr<UpvalInfo>;

struct UpvalInfo
{
	// Register index occupied by local variables if >= 0
	// else captured by upper function
	int locVarSlot;
	// Upvalue table index by upper function if >= 0
	int upvalIndex;
	// Record the order of appearance in the function
	int index;

	UpvalInfo()
	{
		locVarSlot = -1;
		upvalIndex = -1;
		index = 0;
	}
};

struct FuncInfo;
using FuncInfoPtr = std::shared_ptr<FuncInfo>;

struct FuncInfo
{
	std::unordered_map<LuaValue, int> constants;
	int usedRegs;
	int maxRegs;
	int scopeLv;
	// Record the local variables declared inside the function in order
	std::vector<LocVarInfoPtr> locVars;
	// Record the current local variables in effect
	std::unordered_map<String, LocVarInfoPtr> locNames;
	// Break table
	using BreakTableElem = std::vector<int>;
	using BreakTableElemPtr = std::shared_ptr<BreakTableElem>;
	std::vector<BreakTableElemPtr> breaks;
	// Upvalues
	FuncInfoPtr parent;
	std::unordered_map<String, UpvalInfoPtr> upvalues;
	// Bytecode
	std::vector<UInt32> insts;

	FuncInfo()
	{
		usedRegs = 0;
		maxRegs = 0;
		scopeLv = 0;
	}

	int IndexOfConstant(const LuaValue& k);
	int AllocReg();
	void FreeReg();
	int AllocRegs(int n);
	void FreeRegs(int n);

	LocVarInfoPtr _CurrentLocVar(const String& name);
	void AddBreakJmp(int pc);
	void EnterScope(bool breakable);
	int AddLocVar(const String& name);
	int SlotOfLocVar(const String& name);
	void ExitScope();
	void RemoveLocVar(LocVarInfoPtr locVar);

	int IndexOfUpval(const String& name);
};