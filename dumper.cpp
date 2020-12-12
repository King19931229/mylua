#include "compiler/dumper/dumper.h"

void DumpWithIndent(const String& msg, int indent)
{
	int numSpaces = indent * 4;
	String line;
	line.reserve(numSpaces + msg.length() + 1);
	for(int i = 0; i < numSpaces; ++i)
		line += ' ';
	line += msg;
	line += '\n';
	printf("%s", line.c_str());
}

void DumpExp(ExpPtr exp, int indent)
{
	DumpWithIndent("{", indent);

	if(exp->IsA<NilExp>())
	{
		DumpWithIndent("NilExp:", indent + 1);
		NilExp* nilExp = exp->Cast<NilExp>();
		DumpWithIndent(Format::FormatString("Line %d", nilExp->Line), indent + 1);
	}
	if(exp->IsA<TrueExp>())
	{
		DumpWithIndent("TrueExp:", indent + 1);
		TrueExp* trueExp = exp->Cast<TrueExp>();
		DumpWithIndent(Format::FormatString("Line %d", trueExp->Line), indent + 1);
	}
	if(exp->IsA<FalseExp>())
	{
		DumpWithIndent("FalseExp:", indent + 1);
		FalseExp* falseExp = exp->Cast<FalseExp>();
		DumpWithIndent(Format::FormatString("Line %d", falseExp->Line), indent + 1);
	}
	if(exp->IsA<VarargExp>())
	{
		DumpWithIndent("VarargExp:", indent + 1);
		VarargExp* varargExp = exp->Cast<VarargExp>();
		DumpWithIndent(Format::FormatString("Line %d", varargExp->Line), indent + 1);
	}
	if(exp->IsA<IntegerExp>())
	{
		DumpWithIndent("IntegerExp:", indent + 1);
		IntegerExp* intExp = exp->Cast<IntegerExp>();
		DumpWithIndent(Format::FormatString("Line %d", intExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Val %lld", intExp->Val), indent + 1);
	}
	if(exp->IsA<FloatExp>())
	{
		DumpWithIndent("FloatExp:", indent + 1);
		FloatExp* floatExp = exp->Cast<FloatExp>();
		DumpWithIndent(Format::FormatString("Line %d", floatExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Val %f", floatExp->Val), indent + 1);
	}
	if(exp->IsA<StringExp>())
	{
		DumpWithIndent("StringExp:", indent + 1);
		StringExp* strExp = exp->Cast<StringExp>();
		DumpWithIndent(Format::FormatString("Line %d", strExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Val %s", strExp->Val.c_str()), indent + 1);
	}
	if(exp->IsA<NameExp>())
	{
		DumpWithIndent("NameExp:", indent + 1);
		NameExp* nameExp = exp->Cast<NameExp>();
		DumpWithIndent(Format::FormatString("Line %d", nameExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Name %s", nameExp->Name.c_str()), indent + 1);
	}
	if(exp->IsA<UnopExp>())
	{
		DumpWithIndent("UnopExp:", indent + 1);
		UnopExp* unExp = exp->Cast<UnopExp>();
		DumpWithIndent(Format::FormatString("Line %d", unExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Op %s", unExp->Op), indent + 1);
		DumpWithIndent(Format::FormatString("Exp"), indent + 1);
		DumpExp(unExp->Exp, indent + 1);
	}
	if(exp->IsA<BinopExp>())
	{
		DumpWithIndent("BinopExp:", indent + 1);
		BinopExp* binExp = exp->Cast<BinopExp>();
		DumpWithIndent(Format::FormatString("Line %d", binExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Op %s", binExp->Op), indent + 1);
		DumpWithIndent(Format::FormatString("Exp1"), indent + 1);
		DumpExp(binExp->Exp1, indent + 1);
		DumpWithIndent(Format::FormatString("Exp2"), indent + 1);
		DumpExp(binExp->Exp2, indent + 1);
	}
	if(exp->IsA<ConcatExp>())
	{
		DumpWithIndent("ConcatExp:", indent + 1);
		ConcatExp* conExp = exp->Cast<ConcatExp>();
		DumpWithIndent(Format::FormatString("Line %d", conExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("Exps"), indent + 1);
		for(ExpPtr part : conExp->Exps)
		{
			DumpExp(part, indent + 1);
		}
	}
	if(exp->IsA<TableConstructorExp>())
	{
		DumpWithIndent("TableConstructorExp:", indent + 1);
		TableConstructorExp* tableExp = exp->Cast<TableConstructorExp>();
		DumpWithIndent(Format::FormatString("Line %d", tableExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("LastLine %d", tableExp->LastLine), indent + 1);
		DumpWithIndent(Format::FormatString("KeyExps"), indent + 1);
		for(ExpPtr part : tableExp->KeyExps)
		{
			DumpExp(part, indent + 1);
		}
		DumpWithIndent(Format::FormatString("ValExps"), indent + 1);
		for(ExpPtr part : tableExp->ValExps)
		{
			DumpExp(part, indent + 1);
		}
	}
	if(exp->IsA<FuncDefExp>())
	{
		DumpWithIndent("FuncDefExp:", indent + 1);
		FuncDefExp* funcExp = exp->Cast<FuncDefExp>();
		DumpWithIndent(Format::FormatString("Line %d", funcExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("LastLine %d", funcExp->LastLine), indent + 1);
		DumpWithIndent(Format::FormatString("ParList"), indent + 1);
		for(const String& par : funcExp->ParList)
		{
			DumpWithIndent(Format::FormatString("\"%s\"", par.c_str()), indent + 1);
		}
		DumpWithIndent(Format::FormatString("IsVararg %d", funcExp->IsVararg), indent + 1);
		DumpWithIndent(Format::FormatString("Block"), indent + 1);
		DumpBlock(funcExp->Block, indent + 1);
	}
	if(exp->IsA<ParensExp>())
	{
		DumpWithIndent("ParensExp:", indent + 1);
		ParensExp* parensExp = exp->Cast<ParensExp>();
		DumpWithIndent(Format::FormatString("Exp"), indent + 1);
		DumpExp(parensExp->Exp, indent + 1);
	}
	if(exp->IsA<TableAccessExp>())
	{
		DumpWithIndent("TableAccessExp:", indent + 1);
		TableAccessExp* tableExp = exp->Cast<TableAccessExp>();
		DumpWithIndent(Format::FormatString("LastLine %d", tableExp->LastLine), indent + 1);
		DumpWithIndent(Format::FormatString("PrefixExp"), indent + 1);
		DumpExp(tableExp->PrefixExp, indent + 1);
		DumpWithIndent(Format::FormatString("KeyExp"), indent + 1);
		DumpExp(tableExp->KeyExp, indent + 1);
	}
	if(exp->IsA<FuncCallExp>())
	{
		DumpWithIndent("FuncCallExp:", indent + 1);
		FuncCallExp* fcExp = exp->Cast<FuncCallExp>();
		DumpWithIndent(Format::FormatString("Line %d", fcExp->Line), indent + 1);
		DumpWithIndent(Format::FormatString("LastLine %d", fcExp->LastLine), indent + 1);
		DumpWithIndent(Format::FormatString("PrefixExp"), indent + 1);
		DumpExp(fcExp->PrefixExp, indent + 1);
		DumpWithIndent(Format::FormatString("NameExp"), indent + 1);
		DumpExp(fcExp->NameExp, indent + 1);
		DumpWithIndent(Format::FormatString("Args"), indent + 1);
		for(ExpPtr part : fcExp->Args)
		{
			DumpExp(part, indent + 1);
		}
	}
	DumpWithIndent("}", indent);
}

void DumpStat(StatPtr stat, int indent)
{
	DumpWithIndent("{", indent);

	DumpWithIndent("}", indent);
}

void DumpBlock(BlockPtr block, int indent)
{
	DumpWithIndent("{", indent);
	DumpWithIndent("Block:", indent + 1);

	DumpWithIndent(Format::FormatString("LastLine %d", block->LastLine), indent + 1);

	for(StatPtr stat : block->Stats)
		DumpStat(stat, indent + 1);

	for(ExpPtr retExp : block->RetExps)
		DumpExp(retExp, indent + 1);

	DumpWithIndent("}", indent);
}