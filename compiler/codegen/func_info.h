#pragma once
#include "compiler/ast/block.h"
#include "compiler/parser/parser.h"
#include "compiler/lexer/lexer.h"
#include "state/lua_value.h"
#include "vm/opcodes.h"

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
	std::unordered_map<int, int> arithAndBitwiseBinops;
	std::vector<UInt32> insts;

	FuncInfo()
	{
		usedRegs = 0;
		maxRegs = 0;
		scopeLv = 0;

		arithAndBitwiseBinops.insert({TOKEN_OP_ADD, OP_ADD});
		arithAndBitwiseBinops.insert({TOKEN_OP_SUB, OP_SUB});
		arithAndBitwiseBinops.insert({TOKEN_OP_MUL, OP_MUL});
		arithAndBitwiseBinops.insert({TOKEN_OP_MOD, OP_MOD});
		arithAndBitwiseBinops.insert({TOKEN_OP_POW, OP_POW});
		arithAndBitwiseBinops.insert({TOKEN_OP_DIV, OP_DIV});
		arithAndBitwiseBinops.insert({TOKEN_OP_IDIV, OP_IDIV});
		arithAndBitwiseBinops.insert({TOKEN_OP_BAND, OP_BAND});
		arithAndBitwiseBinops.insert({TOKEN_OP_BOR, OP_BOR});
		arithAndBitwiseBinops.insert({TOKEN_OP_BXOR, OP_BXOR});
		arithAndBitwiseBinops.insert({TOKEN_OP_SHL, OP_SHL});
		arithAndBitwiseBinops.insert({TOKEN_OP_SHR, OP_SHR});
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

	int PC();
	void FixSbx(int pc, int sBx);
	void EmitABC(int opcode, int a, int b, int c);
	void EmitABx(int opcode, int a, int bx);
	void EmitAsBx(int opcode, int a, int b);
	void EmitAx(int opcode, int a);
	void EmitMove(int a, int b);
	void EmitLoadNil(int a, int n);
	void EmitLoadBool(int a, int b, int c);
	void EmitLoadK(int a, const LuaValue& k);
	void EmitVararg(int a, int n);
	void EmitClosure(int a, int bx);
	void EmitNewTable(int a, int nArr, int nRec);
	void EmitSetList(int a, int b, int c);
	void EmitGetTable(int a, int b, int c);
	void EmitSetTable(int a, int b, int c);
	void EmitGetUpval(int a, int b);
	void EmitSetUpval(int a, int b);
	void EmitGetTabUp(int a, int b, int c);
	void EmitSetTabUp(int a, int b, int c);
	void EmitCall(int a, int nArgs, int nRet);
	void EmitTailCall(int a, int nArgs);
	void EmitReturn(int a, int n);
	void EmitSelf(int a, int b, int c);
	int EmitJmp(int a, int sBx);
	void EmitTest(int a, int c);
	void EmitTestSet(int a, int b, int c);
	int EmitForPrep(int a, int sBx);
	int EmitForLoop(int a, int sBx);
	void EmitTForCall(int a, int c);
	void EmitTForLoop(int a, int sBx);
	void EmitUnaryOp(int op, int a, int b);
	void EmitBinaryOp(int op, int a, int b, int c);
};