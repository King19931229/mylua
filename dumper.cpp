#include "compiler/dumper/dumper.h"

void DumpWithIndent(const String& msg, int indent, FILE* fp)
{
	int numSpaces = indent * 4;
	String line;
	line.reserve(numSpaces + msg.length() + 1);
	for(int i = 0; i < numSpaces; ++i)
		line += ' ';
	line += msg;
	line += '\n';
	printf("%s", line.c_str());
	if(fp)
	{
		fprintf(fp, "%s", line.c_str());
		fflush(fp);
	}
}

void DumpExp(ExpPtr exp, int indent, FILE* fp)
{
	DumpWithIndent("{", indent, fp);;

	if(exp->IsA<NilExp>())
	{
		DumpWithIndent("NilExp:", indent + 1, fp);
		NilExp* nilExp = exp->Cast<NilExp>();
		DumpWithIndent(Format::FormatString("Line %d", nilExp->Line), indent + 1, fp);
	}
	if(exp->IsA<TrueExp>())
	{
		DumpWithIndent("TrueExp:", indent + 1, fp);
		TrueExp* trueExp = exp->Cast<TrueExp>();
		DumpWithIndent(Format::FormatString("Line %d", trueExp->Line), indent + 1, fp);
	}
	if(exp->IsA<FalseExp>())
	{
		DumpWithIndent("FalseExp:", indent + 1, fp);
		FalseExp* falseExp = exp->Cast<FalseExp>();
		DumpWithIndent(Format::FormatString("Line %d", falseExp->Line), indent + 1, fp);
	}
	if(exp->IsA<VarargExp>())
	{
		DumpWithIndent("VarargExp:", indent + 1, fp);
		VarargExp* varargExp = exp->Cast<VarargExp>();
		DumpWithIndent(Format::FormatString("Line %d", varargExp->Line), indent + 1, fp);
	}
	if(exp->IsA<IntegerExp>())
	{
		DumpWithIndent("IntegerExp:", indent + 1, fp);
		IntegerExp* intExp = exp->Cast<IntegerExp>();
		DumpWithIndent(Format::FormatString("Line %d", intExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Val %lld", intExp->Val), indent + 1, fp);
	}
	if(exp->IsA<FloatExp>())
	{
		DumpWithIndent("FloatExp:", indent + 1, fp);
		FloatExp* floatExp = exp->Cast<FloatExp>();
		DumpWithIndent(Format::FormatString("Line %d", floatExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Val %f", floatExp->Val), indent + 1, fp);
	}
	if(exp->IsA<StringExp>())
	{
		DumpWithIndent("StringExp:", indent + 1, fp);
		StringExp* strExp = exp->Cast<StringExp>();
		DumpWithIndent(Format::FormatString("Line %d", strExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Val %s", strExp->Val.c_str()), indent + 1, fp);
	}
	if(exp->IsA<NameExp>())
	{
		DumpWithIndent("NameExp:", indent + 1, fp);
		NameExp* nameExp = exp->Cast<NameExp>();
		DumpWithIndent(Format::FormatString("Line %d", nameExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Name %s", nameExp->Name.c_str()), indent + 1, fp);
	}
	if(exp->IsA<UnopExp>())
	{
		DumpWithIndent("UnopExp:", indent + 1, fp);
		UnopExp* unExp = exp->Cast<UnopExp>();
		DumpWithIndent(Format::FormatString("Line %d", unExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Op %d", unExp->Op), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Exp"), indent + 1, fp);
		DumpExp(unExp->Exp, indent + 1, fp);
	}
	if(exp->IsA<BinopExp>())
	{
		DumpWithIndent("BinopExp:", indent + 1, fp);
		BinopExp* binExp = exp->Cast<BinopExp>();
		DumpWithIndent(Format::FormatString("Line %d", binExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Op %d", binExp->Op), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Exp1"), indent + 1, fp);
		DumpExp(binExp->Exp1, indent + 1, fp);
		DumpWithIndent(Format::FormatString("Exp2"), indent + 1, fp);
		DumpExp(binExp->Exp2, indent + 1, fp);
	}
	if(exp->IsA<ConcatExp>())
	{
		DumpWithIndent("ConcatExp:", indent + 1, fp);
		ConcatExp* conExp = exp->Cast<ConcatExp>();
		DumpWithIndent(Format::FormatString("Line %d", conExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Exps"), indent + 1, fp);
		for(ExpPtr part : conExp->Exps)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	if(exp->IsA<TableConstructorExp>())
	{
		DumpWithIndent("TableConstructorExp:", indent + 1, fp);
		TableConstructorExp* tableExp = exp->Cast<TableConstructorExp>();
		DumpWithIndent(Format::FormatString("Line %d", tableExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("LastLine %d", tableExp->LastLine), indent + 1, fp);
		DumpWithIndent(Format::FormatString("KeyExps"), indent + 1, fp);
		for(ExpPtr part : tableExp->KeyExps)
		{
			DumpExp(part, indent + 1, fp);
		}
		DumpWithIndent(Format::FormatString("ValExps"), indent + 1, fp);
		for(ExpPtr part : tableExp->ValExps)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	if(exp->IsA<FuncDefExp>())
	{
		DumpWithIndent("FuncDefExp:", indent + 1, fp);
		FuncDefExp* funcExp = exp->Cast<FuncDefExp>();
		DumpWithIndent(Format::FormatString("Line %d", funcExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("LastLine %d", funcExp->LastLine), indent + 1, fp);
		DumpWithIndent(Format::FormatString("ParList"), indent + 1, fp);
		for(const String& par : funcExp->ParList)
		{
			DumpWithIndent(Format::FormatString("%s", par.c_str()), indent + 1, fp);
		}
		DumpWithIndent(Format::FormatString("IsVararg %d", funcExp->IsVararg), indent + 1, fp);
		DumpWithIndent(Format::FormatString("Block"), indent + 1, fp);
		DumpBlock(funcExp->Block, indent + 1, fp);
	}
	if(exp->IsA<ParensExp>())
	{
		DumpWithIndent("ParensExp:", indent + 1, fp);
		ParensExp* parensExp = exp->Cast<ParensExp>();
		DumpWithIndent(Format::FormatString("Exp"), indent + 1, fp);
		DumpExp(parensExp->Exp, indent + 1, fp);
	}
	if(exp->IsA<TableAccessExp>())
	{
		DumpWithIndent("TableAccessExp:", indent + 1, fp);
		TableAccessExp* tableExp = exp->Cast<TableAccessExp>();
		DumpWithIndent(Format::FormatString("LastLine %d", tableExp->LastLine), indent + 1, fp);
		DumpWithIndent(Format::FormatString("PrefixExp"), indent + 1, fp);
		DumpExp(tableExp->PrefixExp, indent + 1, fp);
		DumpWithIndent(Format::FormatString("KeyExp"), indent + 1, fp);
		DumpExp(tableExp->KeyExp, indent + 1, fp);
	}
	if(exp->IsA<FuncCallExp>())
	{
		DumpWithIndent("FuncCallExp:", indent + 1, fp);
		FuncCallExp* fcExp = exp->Cast<FuncCallExp>();
		DumpWithIndent(Format::FormatString("Line %d", fcExp->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("LastLine %d", fcExp->LastLine), indent + 1, fp);
		DumpWithIndent(Format::FormatString("PrefixExp"), indent + 1, fp);
		DumpExp(fcExp->PrefixExp, indent + 1, fp);
		DumpWithIndent(Format::FormatString("NameExp"), indent + 1, fp);
		DumpExp(fcExp->NameExp, indent + 1, fp);
		DumpWithIndent(Format::FormatString("Args"), indent + 1, fp);
		for(ExpPtr part : fcExp->Args)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	DumpWithIndent("}", indent, fp);
}

void DumpStat(StatPtr stat, int indent, FILE* fp)
{
	DumpWithIndent("{", indent, fp);

	if(stat->IsA<EmptyStat>())
	{
		DumpWithIndent("EmptyStat:", indent + 1, fp);
	}
	if(stat->IsA<BreakStat>())
	{
		DumpWithIndent("BreakStat:", indent + 1, fp);
		BreakStat* breakStat = stat->Cast<BreakStat>();
		DumpWithIndent(Format::FormatString("Line %d", breakStat->Line), indent + 1, fp);
	}
	if(stat->IsA<LabelStat>())
	{
		DumpWithIndent("LabelStat:", indent + 1, fp);
		LabelStat* labelStat = stat->Cast<LabelStat>();
		DumpWithIndent(Format::FormatString("Name %s", labelStat->Name.c_str()), indent + 1, fp);
	}
	if(stat->IsA<GotoStat>())
	{
		DumpWithIndent("GotoStat:", indent + 1, fp);
		GotoStat* goStat = stat->Cast<GotoStat>();
		DumpWithIndent(Format::FormatString("Name %s", goStat->Name.c_str()), indent + 1, fp);
	}
	if(stat->IsA<DoStat>())
	{
		DumpWithIndent("DoStat:", indent + 1, fp);
		DoStat* doStat = stat->Cast<DoStat>();
		DumpWithIndent(Format::FormatString("Block"), indent + 1, fp);
		DumpBlock(doStat->Block, indent + 1, fp);
	}
	if(stat->IsA<WhileStat>())
	{
		DumpWithIndent("WhileStat:", indent + 1, fp);
		WhileStat* whileStat = stat->Cast<WhileStat>();
		DumpWithIndent(Format::FormatString("Exp"), indent + 1, fp);
		DumpExp(whileStat->Exp, indent + 1, fp);
		DumpWithIndent(Format::FormatString("Block"), indent + 1, fp);
		DumpBlock(whileStat->Block, indent + 1, fp);
	}
	if(stat->IsA<RepeatStat>())
	{
		DumpWithIndent("RepeatStat:", indent + 1, fp);
		RepeatStat* repeatStat = stat->Cast<RepeatStat>();
		DumpWithIndent(Format::FormatString("Block"), indent + 1, fp);
		DumpBlock(repeatStat->Block, indent + 1, fp);
		DumpWithIndent(Format::FormatString("Exp"), indent + 1, fp);
		DumpExp(repeatStat->Exp, indent + 1, fp);
	}
	if(stat->IsA<IfStat>())
	{
		DumpWithIndent("IfStat:", indent + 1, fp);
		IfStat* ifStat = stat->Cast<IfStat>();
		DumpWithIndent("Exps", indent + 1, fp);
		for(ExpPtr part : ifStat->Exps)
		{
			DumpExp(part, indent + 1, fp);
		}
		DumpWithIndent("Blocks", indent + 1, fp);
		for(BlockPtr part : ifStat->Blocks)
		{
			DumpBlock(part, indent + 1, fp);
		}
	}
	if(stat->IsA<ForNumStat>())
	{
		DumpWithIndent("ForNumStat:", indent + 1, fp);
		ForNumStat* forStat = stat->Cast<ForNumStat>();
		DumpWithIndent(Format::FormatString("LineOfFor %d", forStat->LineOfFor), indent + 1, fp);
		DumpWithIndent(Format::FormatString("LineOfDo %d", forStat->LineOfDo), indent + 1, fp);
		DumpWithIndent(Format::FormatString("VarName %s", forStat->VarName.c_str()), indent + 1, fp);
		DumpWithIndent("InitExp", indent + 1, fp);
		DumpExp(forStat->InitExp, indent + 1, fp);
		DumpWithIndent("LimitExp", indent + 1, fp);
		DumpExp(forStat->LimitExp, indent + 1, fp);
		DumpWithIndent("StepExp", indent + 1, fp);
		DumpExp(forStat->StepExp, indent + 1, fp);
		DumpWithIndent("Block", indent + 1, fp);
		DumpBlock(forStat->Block, indent + 1, fp);
	}
	if(stat->IsA<ForInStat>())
	{
		DumpWithIndent("ForInStat:", indent + 1, fp);
		ForInStat* forStat = stat->Cast<ForInStat>();
		DumpWithIndent(Format::FormatString("LineOfDo %d", forStat->LineOfDo), indent + 1, fp);
		DumpWithIndent(Format::FormatString("NameList"), indent + 1, fp);
		for(const String& name : forStat->NameList)
		{
			DumpWithIndent(Format::FormatString("%s", name.c_str()), indent + 1, fp);
		}
		DumpWithIndent(Format::FormatString("ExpList"), indent + 1, fp);
		for(ExpPtr part : forStat->ExpList)
		{
			DumpExp(part, indent + 1, fp);
		}
		DumpWithIndent("Block", indent + 1, fp);
		DumpBlock(forStat->Block, indent + 1, fp);
	}
	if(stat->IsA<LocalVarDeclList>())
	{
		DumpWithIndent("LocalVarDeclList:", indent + 1, fp);
		LocalVarDeclList* localStat = stat->Cast<LocalVarDeclList>();
		DumpWithIndent(Format::FormatString("NameList"), indent + 1, fp);
		for(const String& name : localStat->NameList)
		{
			DumpWithIndent(Format::FormatString("%s", name.c_str()), indent + 1, fp);
		}
		DumpWithIndent(Format::FormatString("ExpList"), indent + 1, fp);
		for(ExpPtr part : localStat->ExpList)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	if(stat->IsA<AssignStat>())
	{
		DumpWithIndent("AssignStat:", indent + 1, fp);
		AssignStat* localStat = stat->Cast<AssignStat>();
		DumpWithIndent(Format::FormatString("LineOfDo %d", localStat->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("VarList"), indent + 1, fp);
		for(ExpPtr part : localStat->VarList)
		{
			DumpExp(part, indent + 1, fp);
		}
		DumpWithIndent(Format::FormatString("ExpList"), indent + 1, fp);
		for(ExpPtr part : localStat->ExpList)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	if(stat->IsA<LocalFuncDefStat>())
	{
		DumpWithIndent("LocalFuncDefStat:", indent + 1, fp);
		LocalFuncDefStat* localStat = stat->Cast<LocalFuncDefStat>();
		DumpWithIndent(Format::FormatString("Name %s", localStat->Name.c_str()), indent + 1, fp);
		DumpWithIndent("Exp", indent + 1, fp);
		DumpExp(localStat->Exp, indent + 1, fp);
	}
	if(stat->IsA<FuncCallStat>())
	{
		DumpWithIndent("FuncCallStat:", indent + 1, fp);
		FuncCallStat* fcStat = stat->Cast<FuncCallStat>();
		DumpWithIndent(Format::FormatString("Line %d", fcStat->Line), indent + 1, fp);
		DumpWithIndent(Format::FormatString("LastLine %d", fcStat->LastLine), indent + 1, fp);
		DumpWithIndent("PrefixExp", indent + 1, fp);
		DumpExp(fcStat->PrefixExp, indent + 1, fp);
		DumpWithIndent("NameExp", indent + 1, fp);
		DumpExp(fcStat->NameExp, indent + 1, fp);
		DumpWithIndent("Args", indent + 1, fp);
		for(ExpPtr part : fcStat->Args)
		{
			DumpExp(part, indent + 1, fp);
		}
	}
	DumpWithIndent("}", indent, fp);
}

void DumpBlock(BlockPtr block, int indent, FILE* fp)
{
	DumpWithIndent("{", indent, fp);
	DumpWithIndent("Block:", indent + 1, fp);

	DumpWithIndent(Format::FormatString("LastLine %d", block->LastLine), indent + 1, fp);

	for(StatPtr stat : block->Stats)
	{
		DumpStat(stat, indent + 1, fp);
	}

	for(ExpPtr retExp : block->RetExps)
	{
		DumpExp(retExp, indent + 1, fp);
	}

	DumpWithIndent("}", indent, fp);
}