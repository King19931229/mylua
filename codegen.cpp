#include "compiler/codegen/func_info.h"

int FuncInfo::IndexOfConstant(const LuaValue& k)
{
	auto it = constants.find(k);
	if(it != constants.end())
		return it->second;

	int idx = constants.size();
	constants.insert({k, idx});
	return idx;
}

int FuncInfo::AllocReg()
{
	usedRegs += 1;
	if(usedRegs >= 255)
	{
		panic("function or expression needs too many registers");
	}
	if(usedRegs >= maxRegs)
	{
		maxRegs = usedRegs;
	}
	// index begin with 0
	return usedRegs - 1;
}

void FuncInfo::FreeReg()
{
	usedRegs -= 1;
}

int FuncInfo::AllocRegs(int n)
{
	panic_cond(n > 0, "n must > 0");
	for(int i = 0; i < n; ++i)
	{
		AllocReg();
	}
	return usedRegs - n;
}

void FuncInfo::FreeRegs(int n)
{
	panic_cond(n > 0, "n must > 0");
	for(int i = 0; i < n; ++i)
	{
		FreeReg();
	}
}

void FuncInfo::AddBreakJmp(int pc)
{
	for(int i = scopeLv; i >= 0; --i)
	{
		if(breaks[i] != nullptr)
		{
			breaks[i]->emplace_back(pc);
			return;
		}
	}
	panic("<break> at line ? not inside a loop!");
}

void FuncInfo::EnterScope(bool breakable)
{
	scopeLv += 1;
	if(breakable)
	{
		breaks.emplace_back(BreakTableElemPtr(new BreakTableElem()));
	}
	else
	{
		breaks.push_back(nullptr);
	}
}

LocVarInfoPtr FuncInfo::_CurrentLocVar(const String& name)
{
	auto it = locNames.find(name);
	if(it != locNames.end())
		return it->second;
	return nullptr;
}

int FuncInfo::AddLocVar(const String& name)
{
	LocVarInfoPtr newVar = LocVarInfoPtr(new LocVarInfo());
	newVar->name = name;
	newVar->prev = _CurrentLocVar(name);
	newVar->scopeLv = scopeLv;
	newVar->slot = AllocReg();

	locVars.emplace_back(newVar);
	// use [] to handle duplicated keys in unordered_map.
	locNames[name] = newVar;
	return newVar->slot;
}

int FuncInfo::SlotOfLocVar(const String& name)
{
	LocVarInfoPtr locVar = _CurrentLocVar(name);
	if(locVar)
		return locVar->slot;
	return -1;		
}

void FuncInfo::ExitScope()
{
	scopeLv -= 1;

	std::vector<LocVarInfoPtr> locVars;
	locVars.reserve(locNames.size());
	for(auto& pair : locNames)
	{
		locVars.push_back(pair.second);
	}

	for(LocVarInfoPtr locVar : locVars)
	{
		if(locVar->scopeLv > scopeLv)
		{
			RemoveLocVar(locVar);
		}
	}

	BreakTableElemPtr pendingBreakJmps = breaks[breaks.size() - 1];
	breaks.pop_back();
	int a = 0;//todo = GetJmpArgA();
	if(pendingBreakJmps)
	{
		for(int pc : *pendingBreakJmps)
		{
			int sBx = PC() - pc;
			int i = (sBx + MAXARG_sBX) << 14 | a << 6 | OP_JMP;
			insts[pc] = (UInt32)i;
		}
	}
}

void FuncInfo::RemoveLocVar(LocVarInfoPtr locVar)
{
	FreeReg();
	// The deleted local variable does not have the previous variable
	// with the same name
	if(locVar->prev == nullptr)
	{
		locNames.erase(locVar->name);
	}
	// The scope of the last variable with the same name
	// is the same as this variable
	else if(locVar->prev->scopeLv == locVar->scopeLv)
	{
		RemoveLocVar(locVar->prev);
	}
	// The scope depth of the last variable with the same name
	// is less than the scope depth of the current variable
	else
	{
		locNames[locVar->name] = locVar->prev;
	}
}

int FuncInfo::IndexOfUpval(const String& name)
{
	// Can find it without creating one
	{
		auto it = upvalues.find(name);
		if(it != upvalues.end())
		{
			UpvalInfoPtr upVal = it->second;
			return upVal->index;
		}
	}

	if(parent)
	{
		auto it = parent->locNames.find(name);
		// Can be captured in upper function.
		if(it != parent->locNames.end())
		{
			LocVarInfoPtr locVar = it->second;
			locVar->captured = true;

			int idx = (int)upvalues.size();
			UpvalInfoPtr upVal = UpvalInfoPtr(new UpvalInfo());
			upVal->locVarSlot = locVar->slot;
			upVal->upvalIndex = -1;
			upVal->index = idx;

			upvalues[name] = upVal;
			return idx;
		}
		// Can't be captured in upper function. Let the upper function capture it.
		else
		{
			int uvIdx = parent->IndexOfUpval(name);
			int idx = (int)upvalues.size();
			UpvalInfoPtr upVal = UpvalInfoPtr(new UpvalInfo());
			upVal->locVarSlot = -1;
			upVal->upvalIndex = uvIdx;
			upVal->index = idx;

			upvalues[name] = upVal;
			return idx;
		}
	}
	return -1;
}

int FuncInfo::PC()
{
	return (int)insts.size() - 1;
}

void FuncInfo::FixSbx(int pc, int sBx)
{
	UInt32 i = insts[pc];
	// clear sBx
	i = i << 18 >> 18;
	// reset Sbx
	i = i | UInt32(sBx + MAXARG_sBX) << 14;
	insts[pc] = i;
}

void FuncInfo::CGBlock(FuncInfoPtr fi, BlockPtr node)
{
	for(StatPtr stat : node->Stats)
	{
		CGStat(fi, stat);
	}

	if(node->RetExps.size())
	{
		CGRetStat(fi, node->RetExps);
	}
}

void FuncInfo::CGRetStat(FuncInfoPtr fi, const ExpArray& exps)
{
	int nExps = (int)exps.size();
	if(nExps == 0)
	{
		fi->EmitReturn(0, 0);
		return;
	}
	
	bool multRet = IsVarargOrFuncCall(exps[nExps - 1]);
	for(int i = 0; i < nExps; ++i)
	{
		ExpPtr exp = exps[i];
		int r = fi->AllocReg();
		if(i == nExps - 1 && multRet)
		{
			CGExp(fi, exp, r, -1);
		}
		else
		{
			CGExp(fi, exp, r, 1);
		}
	}
	fi->FreeRegs(nExps);

	int a = fi->usedRegs;
	if(multRet)
	{
		// EmitABC(OP_RETURN, a, 0, 0);
		fi->EmitReturn(a, -1);
	}
	else
	{
		fi->EmitReturn(a, nExps);
	}
}

bool FuncInfo::IsVarargOrFuncCall(ExpPtr exp)
{
	if(exp->IsA<VarargExp>() || exp->IsA<FuncCallExp>())
	{
		return true;
	}
	return false;
}

void FuncInfo::CloseOpenUpvals()
{
	int a = GetJmpArgA();
	if(a > 0)
		EmitJmp(a, 0);
}

int FuncInfo::GetJmpArgA()
{
	bool hasCapturedLocVars = false;
	int minSlotOfLocVars = maxRegs;
	// For each local variable name
	for(auto& pair : locNames)
	{
		LocVarInfoPtr locVar = pair.second;
		if(locVar->scopeLv == scopeLv)
		{
			// Loop through all the variables that are on the current scopeLv
			for(LocVarInfoPtr v = locVar; v != nullptr && v->scopeLv == scopeLv; v = v->prev)
			{
				if(v->captured)
				{
					hasCapturedLocVars = true;
				}
				if(v->slot < minSlotOfLocVars && v->name[0] != '(' /*why*/)
				{
					minSlotOfLocVars = v->slot;
				}
			}
		}
	}

	if(hasCapturedLocVars)
	{
		// minSlotOfLocVars is the index of the first closed upvalue
		return minSlotOfLocVars + 1;
	}
	else
	{
		return 0;
	}
}

void FuncInfo::CGStat(FuncInfoPtr fi, StatPtr node)
{
	if(node->IsA<FuncCallStat>())
		CGFuncCall(fi, node);
	else if(node->IsA<BreakStat>())
		CGBreakStat(fi, node);
	else if(node->IsA<DoStat>())
		CGDoStat(fi, node);
	else if(node->IsA<RepeatStat>())
		CGRepeatStat(fi, node);
	else if(node->IsA<WhileStat>())
		CGWhileStat(fi, node);
	else if(node->IsA<IfStat>())
		CGIfStat(fi, node);
	else if(node->IsA<ForNumStat>())
		CGForNumStat(fi, node);
	else if(node->IsA<ForInStat>())
		CGForInStat(fi, node);
	else if(node->IsA<AssignStat>())
		CGAssignStat(fi, node);
	else if(node->IsA<LocalVarDeclStat>())
		CGLocalVarDeclStat(fi, node);
	else if(node->IsA<LocalFuncDefStat>())
		CGLocalFuncDefStat(fi, node);
	else
		panic("not support right now");
}

void FuncInfo::CGLocalFuncDefStat(FuncInfoPtr fi, StatPtr node)
{
	// LocalFuncDefStat* stat = node->Cast<LocalFuncDefStat>();
	// int r = fi->AddLocVar(stat->Name);
	// todo CGFuncDefExp(fi, stat->Exp, r);
}

void FuncInfo::CGFuncCall(FuncInfoPtr fi, StatPtr node)
{
	// int r = fi->AllocReg();
	// todo CGFuncCallExp(fi, node, r, 0);
	// fi->FreeReg();
}

void FuncInfo::CGBreakStat(FuncInfoPtr fi, StatPtr node)
{
	// Add the break instruction first,
	// and then backpatch it when exiting the scope
	int pc = fi->EmitJmp(0, 0);
	fi->AddBreakJmp(pc);
}

void FuncInfo::CGDoStat(FuncInfoPtr fi, StatPtr node)
{
	DoStat* stat = node->Cast<DoStat>();
	fi->EnterScope(false);
	CGBlock(fi, stat->Block);
	fi->CloseOpenUpvals();
	fi->ExitScope();
}

void FuncInfo::CGWhileStat(FuncInfoPtr fi, StatPtr node)
{
	WhileStat* stat = node->Cast<WhileStat>();
	int pcBeforeExp = fi->PC();

	int r = fi->AllocReg();
	CGExp(fi, stat->Exp, r, 1);
	fi->FreeReg();

	// If the r result is true, then skip the next instruction
	fi->EmitTest(r, 0);
	// The jmp instruction will be backpatched later
	int pcJmpToEnd = fi->EmitJmp(0, 0);

	fi->EnterScope(true);
	CGBlock(fi, stat->Block);
	fi->CloseOpenUpvals();
	fi->EmitJmp(0, pcBeforeExp - fi->PC() - 1);
	fi->ExitScope();

	// Now backpatch the jmp instruction
	fi->FixSbx(pcJmpToEnd, fi->PC() - pcJmpToEnd);
}

void FuncInfo::CGRepeatStat(FuncInfoPtr fi, StatPtr node)
{
	RepeatStat* stat = node->Cast<RepeatStat>();	
	fi->EnterScope(true);

	int pcBeforeBlock = fi->PC();
	CGBlock(fi, stat->Block);

	int r = fi->AllocReg();
	CGExp(fi, stat->Exp, r, 1);
	fi->FreeReg();

	// If the r result is true, then skip the next instruction
	fi->EmitTest(r, 0);
	fi->EmitJmp(fi->GetJmpArgA(), pcBeforeBlock - fi->PC() - 1);
	fi->CloseOpenUpvals();

	fi->ExitScope();
}

void FuncInfo::CGIfStat(FuncInfoPtr fi, StatPtr node)
{
	IfStat* stat = node->Cast<IfStat>();

	std::vector<int> pcJmpToEnds;
	pcJmpToEnds.resize(stat->Exps.size());

	int pcJmpToNextExp = -1;
	int nExps = (int)stat->Exps.size();
	for(int i = 0; i < nExps; ++i)
	{
		ExpPtr exp = stat->Exps[i];

		if(pcJmpToNextExp >= 0)
		{
			// Backpatch the previous JmpToNext
			fi->FixSbx(pcJmpToNextExp, fi->PC() - pcJmpToNextExp);
		}

		int r = fi->AllocReg(); CGExp(fi, exp, r, 1); fi->FreeReg();
		fi->EmitTest(r, 0); pcJmpToNextExp = fi->EmitJmp(0, 0);

		fi->EnterScope(false);
		CGBlock(fi, stat->Blocks[i]);
		fi->ExitScope();

		if(i  < (int)(stat->Exps.size() - 1))
			pcJmpToEnds[i] = fi->EmitJmp(0, 0);
		// The last JmpToEnd point to last JmpToNext
		else
			pcJmpToEnds[i] = pcJmpToNextExp;
	}

	// Backpatch all JmpToEnd
	for(int pc : pcJmpToEnds)
	{
		fi->FixSbx(pc, fi->PC() - pc);
	}
}

void FuncInfo::CGForNumStat(FuncInfoPtr fi, StatPtr node)
{
	ForNumStat* stat = node->Cast<ForNumStat>();	
	fi->EnterScope(true);

	StatPtr tempDecl = Stat::New<LocalVarDeclStat>();
	tempDecl->Cast<LocalVarDeclStat>()->NameList = {"(for index)", "(for limit)", "(for step)"};
	tempDecl->Cast<LocalVarDeclStat>()->ExpList = {stat->InitExp, stat->LimitExp, stat->StepExp};
	
	fi->AddLocVar(stat->VarName);
	int a = fi->usedRegs - 4;
	int pcForPrep = fi->EmitForPrep(a, 0);
	CGBlock(fi, stat->Block); fi->CloseOpenUpvals();
	int pcForLoop = fi->EmitForLoop(0, 0);

	// pcForPrep + 1 + k = pcForLoop
	fi->FixSbx(pcForPrep, pcForLoop - pcForPrep - 1);
	// pcForLoop + 1 + k = pcForLoop + 1
	fi->FixSbx(pcForLoop, pcForPrep - pcForLoop);

	fi->ExitScope();
}

void FuncInfo::CGForInStat(FuncInfoPtr fi, StatPtr node)
{
	ForInStat* stat = node->Cast<ForInStat>();	
	fi->EnterScope(true);

	StatPtr tempDecl = Stat::New<LocalVarDeclStat>();
	tempDecl->Cast<LocalVarDeclStat>()->NameList = {"(for generator)", "(for state)", "(for control)"};
	tempDecl->Cast<LocalVarDeclStat>()->ExpList = stat->ExpList;

	for(const String& name : stat->NameList)
	{
		fi->AddLocVar(name);
	}

	int pcJmpToTFC = fi->EmitJmp(0, 0);
	CGBlock(fi, stat->Block); fi->CloseOpenUpvals();
	// pcJmpToTFC + 1 + k = PC() + 1
	fi->FixSbx(pcJmpToTFC, fi->PC() - pcJmpToTFC);

	int rGenerator = fi->SlotOfLocVar("(for generator)");
	fi->EmitTForCall(rGenerator, (int)stat->NameList.size());
	// PC() + 1 + 1 + k = pcJmpToTFC + 1
	fi->EmitTForCall(rGenerator + 2, pcJmpToTFC - fi->PC() - 1);

	fi->ExitScope();
}

void FuncInfo::CGLocalVarDeclStat(FuncInfoPtr fi, StatPtr node)
{

}

void FuncInfo::CGAssignStat(FuncInfoPtr fi, StatPtr node)
{
	
}

void FuncInfo::EmitABC(int opcode, int a, int b, int c)
{
	UInt32 i = b << 23 | c << 14 | a << 6 | opcode;
	insts.emplace_back(i);
}

void FuncInfo::EmitABx(int opcode, int a, int bx)
{
	UInt32 i = bx << 14 | a << 6 | opcode;
	insts.emplace_back(i);
}

void FuncInfo::EmitAsBx(int opcode, int a, int b)
{
	UInt32 i = (b + MAXARG_sBX) << 14 | a << 6 | opcode;
	insts.emplace_back(i);
}

void FuncInfo::EmitAx(int opcode, int a)
{
	UInt32 i = a << 6 | opcode;
	insts.emplace_back(i);
}

// r[a] = r[b]
void FuncInfo::EmitMove(int a, int b)
{
	EmitABC(OP_MOVE, a, b, 0);
}

// r[a], r[a+1], ..., r[a+b] = nil
void FuncInfo::EmitLoadNil(int a, int n)
{
	EmitABC(OP_LOADNIL, a, n - 1, 0);
}

// r[a] = (bool)b; if (c) pc++
void FuncInfo::EmitLoadBool(int a, int b, int c)
{
	EmitABC(OP_LOADBOOL, a, b, c);
}

// r[a] = kst[bx]
void FuncInfo::EmitLoadK(int a, const LuaValue& k)
{
	int idx = IndexOfConstant(k);
	if(idx < (1 << 18))
	{
		EmitABx(OP_LOADKX, a, idx);
	}
	else
	{
		EmitABx(OP_LOADKX, a, 0);
		EmitAx(OP_EXTRAARG, idx);
	}
}

// r[a], r[a+1], ..., r[a+b-2] = vararg
void FuncInfo::EmitVararg(int a, int n)
{
	// b - 1 = n ---> b = n + 1
	// b - 1 = n + 1 - 1 = n
	EmitABC(OP_VARARG, a, n + 1, 0);
}

// r[a], r[a+1], ..., r[a+b-2] = vararg
void FuncInfo::EmitClosure(int a, int bx)
{
	EmitABx(OP_CLOSURE, a, bx);
}

// r[a] = {}
void FuncInfo::EmitNewTable(int a, int nArr, int nRec)
{
	EmitABC(OP_NEWTABLE, a, Int2fb(nArr), Int2fb(nRec));
}

// r[a][(c-1)*FPF+i] := r[a+i], 1 <= i <= b
void FuncInfo::EmitSetList(int a, int b, int c)
{
	EmitABC(OP_SETLIST, a, b, c);
}

// r[a] := r[b][rk(c)]
void FuncInfo::EmitGetTable(int a, int b, int c)
{
	EmitABC(OP_GETTABLE, a, b, c);
}

// r[a][rk(b)] = rk(c)
void FuncInfo::EmitSetTable(int a, int b, int c)
{
	EmitABC(OP_SETTABLE, a, b, c);
}

// r[a] = upval[b]
void FuncInfo::EmitGetUpval(int a, int b)
{
	EmitABC(OP_GETUPVAL, a, b, 0);
}

// upval[b] = r[a]
void FuncInfo::EmitSetUpval(int a, int b)
{
	EmitABC(OP_SETUPVAL, a, b, 0);
}

// r[a] = upval[b][rk(c)]
void FuncInfo::EmitGetTabUp(int a, int b, int c)
{
	EmitABC(OP_GETTABUP, a, b, c);
}

// upval[a][rk(b)] = rk(c)
void FuncInfo::EmitSetTabUp(int a, int b, int c)
{
	EmitABC(OP_SETTABUP, a, b, c);
}

// r[a], ..., r[a+c-2] = r[a](r[a+1], ..., r[a+b-1])
void FuncInfo::EmitCall(int a, int nArgs, int nRet)
{
	// b - 1 - 1 + 1 = nArgs ---> b = nArgs + 1
	// c - 1 = nRet --> c = nRet + 1
	EmitABC(OP_CALL, a, nArgs + 1, nRet + 1);
}

// return r[a], ... ,r[a+b-2]
void FuncInfo::EmitTailCall(int a, int nArgs)
{
	// b - 2 + 1 = nArgs --> b = nArgs + 1
	EmitABC(OP_TAILCALL, a, nArgs + 1, 0);
}

// return r[a], ... ,r[a+b-2]
void FuncInfo::EmitReturn(int a, int n)
{
	// b - 2 + 1 = n --> b = n + 1
	EmitABC(OP_RETURN, a, n + 1, 0);
}

// r[a+1] := r[b]; r[a] := r[b][rk(c)]
void FuncInfo::EmitSelf(int a, int b, int c)
{
	EmitABC(OP_SELF, a, b, c);
}

// pc+=sBx; if (a) close all upvalues >= r[a - 1]
int FuncInfo::EmitJmp(int a, int sBx)
{
	EmitAsBx(OP_JMP, a, sBx);
	return (int)insts.size() - 1;
}

// if not (r[a] <=> c) then pc++
void FuncInfo::EmitTest(int a, int c)
{
	EmitABC(OP_TEST, a, 0, c);
}

// if (r[b] <=> c) then r[a] := r[b] else pc++
void FuncInfo::EmitTestSet(int a, int b, int c)
{
	EmitABC(OP_TESTSET, a, b, c);
}

int FuncInfo::EmitForPrep(int a, int sBx)
{
	EmitAsBx(OP_FORPREP, a, sBx);
	return (int)insts.size() - 1;
}

int FuncInfo::EmitForLoop(int a, int sBx)
{
	EmitAsBx(OP_FORLOOP, a, sBx);
	return (int)insts.size() - 1;
}

void FuncInfo::EmitTForCall(int a, int c)
{
	EmitABC(OP_TFORCALL, a, 0, c);
}

void FuncInfo::EmitTForLoop(int a, int sBx)
{
	EmitAsBx(OP_TFORLOOP, a, sBx);
}

// r[a] = op r[b]
void FuncInfo::EmitUnaryOp(int op, int a, int b)
{
	switch(op)
	{
	case TOKEN_OP_NOT:
		EmitABC(OP_NOT, a, b, 0);
	case TOKEN_OP_BNOT:
		EmitABC(OP_BNOT, a, b, 0);
	case TOKEN_OP_LEN:
		EmitABC(OP_LEN, a, b, 0);
	case TOKEN_OP_UNM:
		EmitABC(OP_UNM, a, b, 0);
	}
}

// r[a] = rk[b] op rk[c]
// arith & bitwise & relational
void FuncInfo::EmitBinaryOp(int op, int a, int b, int c)
{
	auto it = arithAndBitwiseBinops.find(op);
	if(it != arithAndBitwiseBinops.end())
	{
		int opcode = it->second;
		EmitABC(opcode, a, b, c);
	}
	else
	{
		switch(op)
		{
			case TOKEN_OP_EQ:
				EmitABC(OP_EQ, 1, b, c);
			case TOKEN_OP_NE:
				EmitABC(OP_EQ, 0, b, c);
			case TOKEN_OP_LT:
				EmitABC(OP_LT, 1, b, c);
			case TOKEN_OP_GT:
				EmitABC(OP_LT, 1, c, b);
			case TOKEN_OP_LE:
				EmitABC(OP_LE, 1, b, c);
			case TOKEN_OP_GE:
				EmitABC(OP_LE, 1, c, b);
		}
		// Skip the next instruction(r[a] = 0). r[a] = 1
		EmitJmp(0, 1);
		// r[a] = 0, skip the next instruction
		EmitLoadBool(a, 0, 1);
		// r[a] = 1
		EmitLoadBool(a, 1, 0);
	}
}