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
	panic_cond(n >= 0, "n must >= 0");
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

// Assign register to new local variable and connect the
// new local variable with the previous local variable
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

	panic_cond(breaks.size() > 0, "break table is empty");
	BreakTableElemPtr pendingBreakJmps = breaks[breaks.size() - 1];
	if(pendingBreakJmps)
	{
		int a = GetJmpArgA();
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
			if(uvIdx >= 0)
			{
				int idx = (int)upvalues.size();
				UpvalInfoPtr upVal = UpvalInfoPtr(new UpvalInfo());
				upVal->locVarSlot = -1;
				upVal->upvalIndex = uvIdx;
				upVal->index = idx;

				upvalues[name] = upVal;
				return idx;
			}
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

void CGBlock(FuncInfoPtr fi, BlockPtr node)
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

void CGRetStat(FuncInfoPtr fi, const ExpArray& exps)
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

/*
bool IsVarargOrFuncCall(ExpPtr exp)
{
	if(exp->IsA<VarargExp>() || exp->IsA<FuncCallExp>())
	{
		return true;
	}
	return false;
}
*/

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

void CGStat(FuncInfoPtr fi, StatPtr node)
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

void CGLocalFuncDefStat(FuncInfoPtr fi, StatPtr node)
{
	LocalFuncDefStat* stat = node->Cast<LocalFuncDefStat>();
	int r = fi->AddLocVar(stat->Name);
	CGFuncDefExp(fi, stat->Exp, r);
}

void CGFuncCall(FuncInfoPtr fi, StatPtr node)
{
	FuncCallStat* stat = node->Cast<FuncCallStat>();
	ExpPtr exp = Exp::New<FuncCallExp>();

	exp->Cast<FuncCallExp>()->Line = stat->Line;
	exp->Cast<FuncCallExp>()->LastLine = stat->LastLine;
	exp->Cast<FuncCallExp>()->PrefixExp = stat->PrefixExp;
	exp->Cast<FuncCallExp>()->NameExp = stat->NameExp;
	exp->Cast<FuncCallExp>()->Args = stat->Args;

	int r = fi->AllocReg();
	CGFuncCallExp(fi, exp, r, 0);
	fi->FreeReg();
}

void CGBreakStat(FuncInfoPtr fi, StatPtr node)
{
	// Add the break instruction first,
	// and then backpatch it when exiting the scope
	int pc = fi->EmitJmp(0, 0);
	fi->AddBreakJmp(pc);
}

void CGDoStat(FuncInfoPtr fi, StatPtr node)
{
	DoStat* stat = node->Cast<DoStat>();
	fi->EnterScope(false);
	CGBlock(fi, stat->Block);
	fi->CloseOpenUpvals();
	fi->ExitScope();
}

void CGWhileStat(FuncInfoPtr fi, StatPtr node)
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

void CGRepeatStat(FuncInfoPtr fi, StatPtr node)
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

void CGIfStat(FuncInfoPtr fi, StatPtr node)
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

void CGForNumStat(FuncInfoPtr fi, StatPtr node)
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

void CGForInStat(FuncInfoPtr fi, StatPtr node)
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

void CGLocalVarDeclStat(FuncInfoPtr fi, StatPtr node)
{
	LocalVarDeclStat* stat = node->Cast<LocalVarDeclStat>();
	int nExps = (int)stat->ExpList.size();
	int nNames = (int)stat->NameList.size();

	int oldRegs = fi->usedRegs;
	if(nExps == nNames)
	{
		for(ExpPtr exp : stat->ExpList)
		{
			int a = fi->AllocReg();
			CGExp(fi, exp, a, 1);
		}
	}
	else if(nExps > nNames)
	{
		for(int i = 0; i < nExps; ++i)
		{
			ExpPtr exp = stat->ExpList[i];
			int a = fi->AllocReg();
			if(i == nExps - 1 && IsVarargOrFuncCall(exp))
			{
				CGExp(fi, exp, a, 0);
			}
			else
			{
				CGExp(fi, exp, a, 1);
			}
		}
	}
	else
	{
		bool multRet = false;
		for(int i = 0; i < nExps; ++i)
		{
			ExpPtr exp = stat->ExpList[i];
			int a = fi->AllocReg();
			if(i == nExps - 1 && IsVarargOrFuncCall(exp))
			{
				multRet = true;
				int n = nNames - nExps + 1;
				fi->AllocRegs(n - 1);
				CGExp(fi, exp, a, n);
			}
			else
			{
				CGExp(fi, exp, a, 1);
			}
		}

		if(!multRet)
		{
			int n = nNames - nExps;
			int a = fi->AllocRegs(n);
			fi->EmitLoadNil(a, n);
		}
	}
	// Reclaim the register
	fi->usedRegs = oldRegs;
	// Declare variables and bind to the register just used
	for(const String& name : stat->NameList)
	{
		fi->AddLocVar(name);
	}
}

void CGAssignStat(FuncInfoPtr fi, StatPtr node)
{
	AssignStat* stat = node->Cast<AssignStat>();

	int nExps = (int)stat->ExpList.size();
	int nVars = (int)stat->VarList.size();
	int oldRegs = fi->usedRegs;
	std::vector<int> tRegs; tRegs.resize(nVars);
	std::vector<int> kRegs; kRegs.resize(nVars);
	std::vector<int> vRegs; vRegs.resize(nVars);

	for(int i = 0; i < nExps; ++i)
	{
		ExpPtr exp = stat->ExpList[i];
		if(exp->IsA<TableAccessExp>())
		{
			TableAccessExp* taExp = exp->Cast<TableAccessExp>();
			tRegs[i] = fi->AllocReg();
			CGExp(fi, taExp->PrefixExp, tRegs[i], 1);
			kRegs[i] = fi->AllocReg();
			CGExp(fi, taExp->KeyExp, kRegs[i], 1);
		}
	}
	// Although there is no register allocated,
	// the position can be occupied first and will be allocated later
	for(int i = 0; i < nVars; ++i)
	{
		vRegs[i] = fi->usedRegs + i;
	}

	if(nExps >= nVars)
	{
		for(int i = 0; i < nExps; ++i)
		{
			ExpPtr exp = stat->ExpList[i];
			int a = fi->AllocReg();
			// nExps > nVars and the last expression is a ... or funccall
			if(i >= nVars && i == nExps - 1 && IsVarargOrFuncCall(exp))
			{
				CGExp(fi, exp, a, 0);
			}
			else
			{
				CGExp(fi, exp, a, 1);
			}
		}
	}
	else
	{
		bool multRet = false;
		for(int i = 0; i < nExps; ++i)
		{
			ExpPtr exp = stat->ExpList[i];
			int a = fi->AllocReg();
			if(i == nExps - 1 && IsVarargOrFuncCall(exp))
			{
				multRet = true;
				int n = nVars - nExps + 1;
				fi->AllocRegs(n - 1);
				CGExp(fi, exp, a, n);
			}
			else
			{
				CGExp(fi, exp, a, 1);
			}
		}
		if(!multRet)
		{
			int n = nVars - nExps;
			int a = fi->AllocRegs(n);
			fi->EmitLoadNil(a, n);
		}
	}

	for(int i = 0; i < nVars; ++i)
	{
		ExpPtr exp = stat->VarList[i];
		if(exp->IsA<NameExp>())
		{
			NameExp* nameExp = exp->Cast<NameExp>();
			const String& varName = nameExp->Name;
			int a = fi->SlotOfLocVar(varName);
			if(a >= 0)
			{
				// Local variable, copy the value of the expression calculated
				// in the register to the register corresponding to the local variable
				fi->EmitMove(a, vRegs[i]);
			}
			else
			{
				int b = fi->IndexOfUpval(varName);
				if(b >= 0)
				{
					
					// Upvalue variable, copy the value of the expression calculated
					// in the register to the corresponding Upvalue table
					fi->EmitSetUpval(vRegs[i], b);
				}
				else
				{
					int a = fi->IndexOfUpval("_ENV");
					int b = 0x100 + fi->IndexOfConstant(LuaValue(varName));
					// Global variable, copy the value of the expression calculated
					// in the register to the corresponding global variable table
					fi->EmitSetTabUp(a, b, vRegs[i]);
				}
			}
		}
	}

	fi->usedRegs = oldRegs;
}

// Put at most n values ​​of expression on register a
void CGExp(FuncInfoPtr fi, ExpPtr node, int a, int n)
{
	if(node->IsA<NilExp>())
		fi->EmitLoadNil(a, n);
	else if(node->IsA<FalseExp>())
		fi->EmitLoadBool(a, 0, 0);
	else if(node->IsA<TrueExp>())
		fi->EmitLoadBool(a, 1, 0);
	else if(node->IsA<IntegerExp>())
		fi->EmitLoadK(a, LuaValue(node->Cast<IntegerExp>()->Val));
	else if(node->IsA<FloatExp>())
		fi->EmitLoadK(a, LuaValue(node->Cast<FloatExp>()->Val));
	else if(node->IsA<StringExp>())
		fi->EmitLoadK(a, LuaValue(node->Cast<StringExp>()->Val));
	else if(node->IsA<ParensExp>())
		CGExp(fi, node->Cast<ParensExp>()->Exp, a, 1);
	else if(node->IsA<VarargExp>())
		CGVarargExp(fi, node, a, n);
	else if(node->IsA<TableConstructorExp>())
		CGTableConstructorExp(fi, node, a);
	else if(node->IsA<UnopExp>())
		CGUnopExp(fi, node, a);
	else if(node->IsA<BinopExp>())
		CGBinopExp(fi, node, a);
	else if(node->IsA<ConcatExp>())
		CGConcatExp(fi, node, a);
	else if(node->IsA<NameExp>())
		CGNameExp(fi, node, a);
	else if(node->IsA<TableAccessExp>())
		CGTableAceessExp(fi, node, a);
	else if(node->IsA<FuncCallExp>())
		CGFuncCallExp(fi, node, a, n);
}

// n pass >=0 means read n parameters, n pass -1 means read all parameters
void CGVarargExp(FuncInfoPtr fi, ExpPtr node, int a, int n)
{
	if(!fi->isVararg)
	{
		panic("cannot use \'...\' outside a vararg function");
	}
	fi->EmitVararg(a, n);
}

void CGFuncDefExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	FuncDefExp* exp = node->Cast<FuncDefExp>();
	FuncInfoPtr subFI = NewFuncInfo(fi, exp);
	fi->subFuncs.emplace_back(subFI);

	for(const String& param : exp->ParList)
	{
		subFI->AddLocVar(param);
	}

	CGBlock(subFI, exp->Block);
	// Exit the scope 0, clean up all local variables
	subFI->ExitScope();
	subFI->EmitReturn(0, 0);

	int bx = (int)fi->subFuncs.size() - 1;
	fi->EmitClosure(a, bx);
}

void CGTableConstructorExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	TableConstructorExp* exp = node->Cast<TableConstructorExp>();
	int nArr = 0;
	for(ExpPtr keyExp : exp->KeyExps)
	{
		if(keyExp == nullptr)
			++nArr;
	}
	int nExps = (int)exp->KeyExps.size();
	bool multRet = nExps > 0 && IsVarargOrFuncCall(exp->ValExps[nExps - 1]);

	fi->EmitNewTable(a, nArr, nExps - nArr);

	int arrIdx = 0;
	for(int i = 0; i < nExps; ++i)
	{
		ExpPtr keyExp = exp->KeyExps[i];
		ExpPtr valExp = exp->ValExps[i];
		// array
		if(keyExp == nullptr)
		{
			++arrIdx;
			int tmp = fi->AllocReg();
			if(i == nExps - 1 && multRet)
			{
				CGExp(fi, valExp, tmp, -1);
			}
			else
			{
				CGExp(fi, valExp, tmp, 1);
			}

			if(arrIdx % LFIELDS_PER_FLUSH == 0 || arrIdx == nArr)
			{
				int n = arrIdx % LFIELDS_PER_FLUSH;
				if(n == 0)
					n = LFIELDS_PER_FLUSH;
				int c = (arrIdx - 1) / LFIELDS_PER_FLUSH + 1;
				fi->FreeRegs(n);
				if(i == nExps - 1 && multRet)
				{
					fi->EmitSetList(a, 0, c);
				}
				else
				{
					fi->EmitSetList(a, n, c);
				}
			}
			continue;
		}

		int b = fi->AllocReg();
		CGExp(fi, keyExp, b, 1);
		int c = fi->AllocReg();
		CGExp(fi, valExp, c, 1);
		fi->FreeRegs(2);
		// a[b] = c
		fi->EmitSetTable(a, b, c);
	}
}

void CGUnopExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	UnopExp* exp = node->Cast<UnopExp>();
	int b = fi->AllocReg();
	CGExp(fi, exp->Exp, b, 1);
	fi->EmitUnaryOp(exp->Op, a, b);
	fi->FreeReg();
}

void CGBinopExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	BinopExp* exp = node->Cast<BinopExp>();
	switch (exp->Op)
	{
		case TOKEN_OP_AND:
		case TOKEN_OP_OR:
		{
			int b = fi->AllocReg();
			CGExp(fi, exp->Exp1, b, 1);
			fi->FreeReg();

			if(exp->Op == TOKEN_OP_AND)
			{
				// r[b] == 0 ? r[a] = r[b] : pc++
				fi->EmitTestSet(a, b, 0);
			}
			else
			{
				// r[b] == 1 ? r[a] = r[b] : pc++
				fi->EmitTestSet(a, b, 1);
			}

			int pcOfJmp = fi->EmitJmp(0, 0);

			b = fi->AllocReg();
			CGExp(fi, exp->Exp2, b, 1);
			fi->FreeReg(); 

			fi->EmitMove(a, b);
			fi->FixSbx(pcOfJmp, fi->PC() - pcOfJmp);

			break;
		}
		default:
		{
			int b = fi->AllocReg();
			CGExp(fi, exp->Exp1, b, 1);
			int c = fi->AllocReg();
			CGExp(fi, exp->Exp2, c, 1);
			fi->EmitBinaryOp(exp->Op, a, b, c);
			fi->FreeRegs(2);
		}
	}
}

void CGConcatExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	ConcatExp* exp = node->Cast<ConcatExp>();
	for(ExpPtr subExp : exp->Exps)
	{
		int _a = fi->AllocReg();
		CGExp(fi, subExp, _a, 1);
	}

	int c = fi->usedRegs - 1;
	int b = c - (int)exp->Exps.size() + 1;
	fi->FreeRegs(c - b + 1);
	fi->EmitABC(OP_CONCAT, a, b, c);
}

void CGNameExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	NameExp* exp = node->Cast<NameExp>();
	int r = fi->SlotOfLocVar(exp->Name);
	if(r >= 0)
	{
		fi->EmitMove(a, r);
	}
	else
	{
		int idx = fi->IndexOfUpval(exp->Name);
		if(idx >= 0)
		{
			// LUA_REGISTRYINDEX - 1 - idx
			fi->EmitGetUpval(a, idx);
		}
		else
		{
			ExpPtr tabExp = Exp::New<TableAccessExp>();
			ExpPtr prefixExp = Exp::New<NameExp>();
			ExpPtr keyExp = Exp::New<StringExp>();

			prefixExp->Cast<NameExp>()->Line = 0;
			prefixExp->Cast<NameExp>()->Name = "_ENV";

			keyExp->Cast<StringExp>()->Line = 0;
			keyExp->Cast<StringExp>()->Val = exp->Name;

			tabExp->Cast<TableAccessExp>()->PrefixExp = prefixExp;
			tabExp->Cast<TableAccessExp>()->KeyExp = keyExp;

			CGTableAceessExp(fi, tabExp, a);
		}
	}
}

void CGTableAceessExp(FuncInfoPtr fi, ExpPtr node, int a)
{
	TableAccessExp* exp = node->Cast<TableAccessExp>();
	int b = fi->AllocReg();
	CGExp(fi, exp->PrefixExp, b, 1);
	int c = fi->AllocReg();
	CGExp(fi, exp->KeyExp, c, 1);
	fi->EmitGetTable(a, b, c);
	fi->FreeRegs(2);
}

int PrepFuncCall(FuncInfoPtr fi, FuncCallExp* node, int a)
{
	int nArgs = (int)node->Args.size();
	bool lastArgIsVarargOrFuncCall = false;

	CGExp(fi, node->PrefixExp, a, 1);
	if(node->NameExp != nullptr)
	{
		StringExp* nameExp = node->NameExp->Cast<StringExp>();
		int c = 0x100 + fi->IndexOfConstant(LuaValue(nameExp->Val));
		int tmp = fi->AllocReg();
		panic_cond(tmp == a + 1, "the register index of self parameter"
			"is not equal to the register index of function + 1");
		fi->EmitSelf(a, a, c);
	}

	for(int i = 0; i < nArgs; ++i)
	{
		int tmp = fi->AllocReg();
		ExpPtr arg = node->Args[i];
		if(i == nArgs - 1 && IsVarargOrFuncCall(arg))
		{
			lastArgIsVarargOrFuncCall = true;
			CGExp(fi, arg, tmp, -1);
		}
		else
		{
			CGExp(fi, arg, tmp, 1);
		}
	}
	fi->FreeRegs(nArgs);

	if(node->NameExp != nullptr)
	{
		fi->FreeReg();
		++nArgs;
	}
	if(lastArgIsVarargOrFuncCall)
	{
		nArgs = -1;
	}
	return nArgs;
}

void CGFuncCallExp(FuncInfoPtr fi, ExpPtr node, int a, int n)
{
	int nArgs = PrepFuncCall(fi, node->Cast<FuncCallExp>(), a);
	fi->EmitCall(a, nArgs, n);
}

void FuncInfo::EmitABC(int opcode, int a, int b, int c)
{
	UInt32 i = b << 23 | c << 14 | a << 6 | opcode;
	insts.emplace_back(i);
#ifdef DEBUG_PRINT_ENABLE
	Instruction inst = Instruction(i);
	DEBUG_PRINT("%s", inst.OpName().c_str());
	Prototype::PrintOperands(inst);
	puts("");
#endif
}

void FuncInfo::EmitABx(int opcode, int a, int bx)
{
	UInt32 i = bx << 14 | a << 6 | opcode;
	insts.emplace_back(i);
#ifdef DEBUG_PRINT_ENABLE
	Instruction inst = Instruction(i);
	DEBUG_PRINT("%s", inst.OpName().c_str());
	Prototype::PrintOperands(inst);
	puts("");
#endif
}

void FuncInfo::EmitAsBx(int opcode, int a, int b)
{
	UInt32 i = (b + MAXARG_sBX) << 14 | a << 6 | opcode;
	insts.emplace_back(i);
#ifdef DEBUG_PRINT_ENABLE
	Instruction inst = Instruction(i);
	DEBUG_PRINT("%s", inst.OpName().c_str());
	Prototype::PrintOperands(inst);
	puts("");
#endif
}

void FuncInfo::EmitAx(int opcode, int a)
{
	UInt32 i = a << 6 | opcode;
	insts.emplace_back(i);
#ifdef DEBUG_PRINT_ENABLE
	Instruction inst = Instruction(i);
	DEBUG_PRINT("%s", inst.OpName().c_str());
	Prototype::PrintOperands(inst);
	puts("");
#endif
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
		EmitABx(OP_LOADK, a, idx);
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
	// b - 1 = n --> b = n + 1
	// b - 1 = n + 1 - 1 = n
	EmitABC(OP_VARARG, a, n + 1, 0);
}

// r[a] = emitClosure(proto[bx])
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
	// b - 1 - 1 + 1 = nArgs --> b = nArgs + 1
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

FuncInfoPtr NewFuncInfo(FuncInfoPtr parent, FuncDefExp* fd)
{
	FuncInfoPtr funcInfo = FuncInfoPtr(new FuncInfo());
	funcInfo->parent = parent;
	funcInfo->isVararg = fd->IsVararg;
	funcInfo->numParams = (int)fd->ParList.size();
	return funcInfo;
}

std::vector<PrototypePtr> ToProtos(const std::vector<FuncInfoPtr>& fis)
{
	std::vector<PrototypePtr> protos;
	protos.resize(fis.size());
	for(size_t i = 0; i < fis.size(); ++i)
	{
		protos[i] = ToProto(fis[i]);
	}
	return protos;
}

Constant GetConstant(const LuaValue& val)
{
	Constant constant;
	if(val.IsBool())
	{
		constant.tag = TAG_BOOLEAN;
		constant.boolean = val.boolean;
	}
	else if(val.IsFloat64())
	{
		constant.tag = TAG_NUMBER;
		constant.luaNum = val.number;
	}
	else if(val.IsInt64())
	{
		constant.tag = TAG_INTEGER;
		constant.luaInteger = val.integer;
	}
	else if(val.IsString())
	{
		if(val.str.length() > 253)
			constant.tag = TAG_LONG_STR;
		else
			constant.tag = TAG_SHORT_STR;
		constant.str = val.str;
	}
	else
	{
		constant.tag = TAG_NIL;
	}
	return constant;
}

std::vector<Constant> GetConstants(FuncInfoPtr fi)
{
	std::vector<Constant> consts;
	consts.resize(fi->constants.size());
	for(auto& pair : fi->constants)
	{
		const LuaValue& k = pair.first;
		int idx = pair.second;
		Constant constant = GetConstant(k);
		consts[idx] = constant;
	}
	return consts;
}

std::vector<Upvalue> GetUpvalues(FuncInfoPtr fi)
{
	std::vector<Upvalue> upvals;
	upvals.resize(fi->upvalues.size());
	for(auto& pair : fi->upvalues)
	{
		UpvalInfoPtr uv = pair.second;
		// instack
		if(uv->locVarSlot >= 0)
			upvals[uv->index] = Upvalue(1, (Byte)uv->locVarSlot);
		else
			upvals[uv->index] = Upvalue(0, (Byte)uv->upvalIndex);
	}
	return upvals;
}

PrototypePtr ToProto(FuncInfoPtr fi)
{
	PrototypePtr proto = PrototypePtr(new Prototype());
	proto->NumParams = (Byte)fi->numParams;
	proto->MaxStackSize = (Byte)fi->maxRegs;
	proto->Code = fi->insts;
	proto->Constants = GetConstants(fi);
	proto->Upvalues = GetUpvalues(fi);
	proto->Protos = ToProtos(fi->subFuncs);
	proto->LineInfo = {};// debug
	proto->LocVars = {};// debug
	proto->UpvalueNames = {};// debug
	proto->IsVararg = fi->isVararg;
	return proto;
}

PrototypePtr GenProto(BlockPtr chunk)
{
	ExpPtr exp = Exp::New<FuncDefExp>();
	FuncDefExp* fd = exp->Cast<FuncDefExp>();
	// Fake main function which contains _ENV
	// 	function __main(...)
	// 		_ENV
	// 		chunk -> main()
	// end
	fd->IsVararg = true;
	fd->Block = chunk;
	FuncInfoPtr fi = NewFuncInfo(nullptr, fd);
	fi->AddLocVar("_ENV");
	CGFuncDefExp(fi, exp, 0);
	return ToProto(fi->subFuncs[0]);
}