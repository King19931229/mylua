#include "compiler/codegen/func_info.h"
#include "vm/opcodes.h"

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
			int sBx = /*todo PC()*/ - pc;
			int i = (sBx + MAXARG_sBX) << 14 | a << 6 | OP_JMP;
			// todo insts[pc] = (UInt32)i;
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