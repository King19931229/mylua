#pragma once
#include "compiler/ast/block.h"
#include "compiler/parser/parser.h"
#include "compiler/lexer/lexer.h"
#include "state/lua_value.h"
#include "vm/opcodes.h"
#include "binchunk/binary_chunk.h"

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
	//
	std::vector<FuncInfoPtr> subFuncs;
	int numParams;
	bool isVararg;

	FuncInfo()
	{
		usedRegs = 0;
		maxRegs = 0;
		scopeLv = 0;
		numParams = 0;
		isVararg = false;

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

	void CloseOpenUpvals();
	int GetJmpArgA();

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

bool IsVarargOrFuncCall(ExpPtr exp);
void CGBlock(FuncInfoPtr fi, BlockPtr node);
void CGRetStat(FuncInfoPtr fi, const ExpArray& exps);
void CGStat(FuncInfoPtr fi, StatPtr node);
void CGLocalFuncDefStat(FuncInfoPtr fi, StatPtr node);
void CGFuncCall(FuncInfoPtr fi, StatPtr node);
void CGBreakStat(FuncInfoPtr fi, StatPtr node);
void CGDoStat(FuncInfoPtr fi, StatPtr node);
void CGWhileStat(FuncInfoPtr fi, StatPtr node);
void CGRepeatStat(FuncInfoPtr fi, StatPtr node);
void CGIfStat(FuncInfoPtr fi, StatPtr node);
void CGForNumStat(FuncInfoPtr fi, StatPtr node);
void CGForInStat(FuncInfoPtr fi, StatPtr node);
void CGLocalVarDeclStat(FuncInfoPtr fi, StatPtr node);
void CGAssignStat(FuncInfoPtr fi, StatPtr node);

void CGExp(FuncInfoPtr fi, ExpPtr node, int a, int n);
void CGVarargExp(FuncInfoPtr fi, ExpPtr node, int a, int n);
void CGFuncDefExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGTableConstructorExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGUnopExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGBinopExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGConcatExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGNameExp(FuncInfoPtr fi, ExpPtr node, int a);
void CGTableAceessExp(FuncInfoPtr fi, ExpPtr node, int a);
int PrepFuncCall(FuncInfoPtr fi, FuncCallExp* node, int a);
void CGFuncCallExp(FuncInfoPtr fi, ExpPtr node, int a, int n);

FuncInfoPtr NewFuncInfo(FuncInfoPtr parent, FuncDefExp* fd);
std::vector<PrototypePtr> ToProtos(const std::vector<FuncInfoPtr>& fis);
Constant GetConstant(const LuaValue& val);
std::vector<Constant> GetConstants(FuncInfoPtr fi);
std::vector<Upvalue> GetUpvalues(FuncInfoPtr fi);
PrototypePtr ToProto(FuncInfoPtr fi);

PrototypePtr GenProto(BlockPtr chunk);