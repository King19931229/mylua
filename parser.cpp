#include "compiler/parser/parser_block.h"
#include "compiler/parser/parser_exp.h"
#include "compiler/ast/stat.h"
#include "compiler/parser/optimizer.h"
#include "number/parser.h"
#include "number/math.h"

#define Error(...)\
{\
	String msg = Format::FormatString(__VA_ARGS__);\
	panic(msg.c_str());\
}

// stats

StatPtr ParseEmptyStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_SEP_SEMI);
	return Stat::New<EmptyStat>();
}

StatPtr ParseBreakStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_BREAK);
	int line = lexer->Line();

	StatPtr _break = Stat::New<BreakStat>();
	_break->Cast<BreakStat>()->Line = line;
	return _break;
}

StatPtr ParseLabelStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_SEP_LABEL);
	String name = lexer->NextIdentifier().token;
	lexer->NextTokenKind(TOKEN_SEP_LABEL);

	StatPtr label = Stat::New<LabelStat>();
	label->Cast<LabelStat>()->Name = name;
	return label;
}

StatPtr ParseGotoStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_GOTO);
	String name = lexer->NextIdentifier().token;

	StatPtr _goto = Stat::New<GotoStat>();
	_goto->Cast<GotoStat>()->Name = name;
	return _goto;
}

StatPtr ParseDoStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_DO);
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_KW_END);
	
	StatPtr _do = Stat::New<DoStat>();
	_do->Cast<DoStat>()->Block = block;
	return _do;
}

StatPtr ParseWhileStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_WHILE);
	ExpPtr exp = ParseExp(lexer);
	lexer->NextTokenKind(TOKEN_KW_DO);
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_KW_END);

	StatPtr _while = Stat::New<WhileStat>();
	_while->Cast<WhileStat>()->Exp = exp;
	_while->Cast<WhileStat>()->Block = block;
	return _while;	
}

StatPtr ParseRepeatStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_REPEAT);
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_KW_UNTIL);
	ExpPtr exp = ParseExp(lexer);

	StatPtr _repeat = Stat::New<RepeatStat>();
	_repeat->Cast<RepeatStat>()->Block = block;
	_repeat->Cast<RepeatStat>()->Exp = exp;
	return _repeat;	
}

StatPtr ParseIfStat(LexerPtr lexer)
{
	ExpArray exps;
	BlockArray blocks;

	lexer->NextTokenKind(TOKEN_KW_IF);
	exps.emplace_back(ParseExp(lexer));
	blocks.emplace_back(ParseBlock(lexer));

	while(lexer->LookAhead() == TOKEN_KW_ELSEIF)
	{
		lexer->NextToken();
		exps.emplace_back(ParseExp(lexer));
		lexer->NextTokenKind(TOKEN_KW_THEN);
		blocks.emplace_back(ParseBlock(lexer));
	}

	if(lexer->LookAhead() == TOKEN_KW_ELSE)
	{
		lexer->NextToken();

		ExpPtr _true = Exp::New<TrueExp>();
		_true->Cast<TrueExp>()->Line = lexer->Line();

		exps.emplace_back(_true);
		blocks.emplace_back(ParseBlock(lexer));
	}

	lexer->NextTokenKind(TOKEN_KW_END);

	StatPtr _if = Stat::New<IfStat>();
	_if->Cast<IfStat>()->Exps = std::move(exps);
	_if->Cast<IfStat>()->Blocks = std::move(blocks);
	return _if;
}

StatPtr _FinishForNumStat(LexerPtr lexer, int lineOfFor, const String& varName)
{
	lexer->NextTokenKind(TOKEN_OP_ASSIGN);
	ExpPtr initExp = ParseExp(lexer);
	lexer->NextTokenKind(TOKEN_SEP_COMMA);
	ExpPtr limitExp = ParseExp(lexer);
	
	ExpPtr stepExp;
	if(lexer->LookAhead() == TOKEN_SEP_COMMA)
	{
		lexer->NextToken();
		stepExp = ParseExp(lexer);
	}
	else
	{
		stepExp = Exp::New<IntegerExp>();
		stepExp->Cast<IntegerExp>()->Line = lexer->Line();
		stepExp->Cast<IntegerExp>()->Val = 1;
	}
	
	int lineOfDo = lexer->NextTokenKind(TOKEN_KW_DO).line;
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_KW_END);

	StatPtr forNumStat = Stat::New<ForNumStat>();
	forNumStat->Cast<ForNumStat>()->LineOfFor = lineOfFor;
	forNumStat->Cast<ForNumStat>()->LineOfDo = lineOfDo;
	forNumStat->Cast<ForNumStat>()->VarName = varName;
	forNumStat->Cast<ForNumStat>()->InitExp = initExp;
	forNumStat->Cast<ForNumStat>()->LimitExp = limitExp;
	forNumStat->Cast<ForNumStat>()->StepExp = stepExp;
	forNumStat->Cast<ForNumStat>()->Block = block;
	return forNumStat;
}

StringArray _FinishNameList(LexerPtr lexer, const String& name0)
{
	StringArray names = {name0};
	while(lexer->LookAhead() == TOKEN_SEP_COMMA)
	{
		lexer->NextToken();
		names.emplace_back(lexer->NextIdentifier().token);
	}
	return names;
}

StatPtr _FinishForInStat(LexerPtr lexer, const String& name0)
{
	StringArray nameList = _FinishNameList(lexer, name0);
	lexer->NextTokenKind(TOKEN_KW_IN);
	ExpArray expList = ParseExpList(lexer);
	int lineOfDo = lexer->NextTokenKind(TOKEN_KW_DO).line;
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_KW_END);

	StatPtr forInStat = Stat::New<ForInStat>();
	forInStat->Cast<ForInStat>()->LineOfDo = lineOfDo;
	forInStat->Cast<ForInStat>()->NameList = std::move(nameList);
	forInStat->Cast<ForInStat>()->ExpList = std::move(expList);
	forInStat->Cast<ForInStat>()->Block = block;
	return forInStat;
}

StatPtr ParseForStat(LexerPtr lexer)
{
	int lineOfFor = lexer->NextTokenKind(TOKEN_KW_FOR).line;
	String name = lexer->NextIdentifier().token;
	if(lexer->LookAhead() == TOKEN_OP_ASSIGN)
	{
		return _FinishForNumStat(lexer, lineOfFor, name);
	}
	else
	{
		return _FinishForInStat(lexer, name);
	}
}

StatPtr _FinishLocalFuncDefStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_FUNCTION);
	String name = lexer->NextIdentifier().token;
	// function body
	ExpPtr fdExp;// todo = ParseFuncDefExp(lexer);

	StatPtr funcDef = Stat::New<LocalFuncDefStat>();
	funcDef->Cast<LocalFuncDefStat>()->Name = name;
	funcDef->Cast<LocalFuncDefStat>()->Exp = fdExp;
	return funcDef;
}

StatPtr _FinishLocalVarDeclStat(LexerPtr lexer)
{
	String name0 = lexer->NextIdentifier().token;
	StringArray nameList = _FinishNameList(lexer, name0);
	ExpArray expList;
	if(lexer->LookAhead() == TOKEN_OP_ASSIGN)
	{
		lexer->NextToken();
		expList = ParseExpList(lexer);
	}
	int lastLine = lexer->Line();

	StatPtr varDecl = Stat::New<LocalVarDeclList>();
	varDecl->Cast<LocalVarDeclList>()->Line = lastLine;
	varDecl->Cast<LocalVarDeclList>()->NameList = std::move(nameList);
	varDecl->Cast<LocalVarDeclList>()->ExpList = std::move(expList);
	return varDecl;
}

StatPtr ParseFuncDefStat(LexerPtr lexer)
{
	// todo
	return ParseEmptyStat(lexer);
}

StatPtr ParseLocalAssignOrFuncDefStat(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_KW_LOCAL);
	if(lexer->LookAhead() == TOKEN_KW_FUNCTION)
	{
		return _FinishLocalFuncDefStat(lexer);
	}
	else
	{
		return _FinishLocalVarDeclStat(lexer);
	}
}

ExpPtr _CheckVar(LexerPtr lexer, ExpPtr exp)
{
	if(exp->IsA<NameExp>() || exp->IsA<TableAccessExp>())
	{
		return exp;
	}
	lexer->NextTokenKind(-1);
	Error("unreachable!");
	return nullptr;
}

ExpArray _FinishVarList(LexerPtr lexer, ExpPtr var0)
{
	ExpArray vars = {var0};
	while(lexer->LookAhead() == TOKEN_SEP_COMMA)
	{
		lexer->NextToken();
		ExpPtr exp = ParsePrefixExp(lexer);
		vars.emplace_back(_CheckVar(lexer, exp));
	}
	return vars;
}

StatPtr ParseAssignStat(LexerPtr lexer, ExpPtr var0)
{
	ExpArray vars = _FinishVarList(lexer, var0);
	lexer->NextTokenKind(TOKEN_OP_ASSIGN);
	ExpArray expList = ParseExpList(lexer);
	int lastLine = lexer->Line();

	StatPtr assign = Stat::New<AssignStat>();
	assign->Cast<AssignStat>()->Line = lastLine;
	assign->Cast<AssignStat>()->VarList = std::move(vars);
	assign->Cast<AssignStat>()->ExpList = std::move(expList);
	return assign;
}

StatPtr ParseAssignOrFuncCallStat(LexerPtr lexer)
{
	ExpPtr prefixExp = ParsePrefixExp(lexer);
	if(prefixExp->IsA<FuncCallExp>())
	{
		StatPtr fc = Stat::New<FuncCallStat>();

		fc->Cast<FuncCallStat>()->Line = prefixExp->Cast<FuncCallExp>()->Line;
		fc->Cast<FuncCallStat>()->LastLine = prefixExp->Cast<FuncCallExp>()->LastLine;
		fc->Cast<FuncCallStat>()->PrefixExp = prefixExp->Cast<FuncCallExp>()->PrefixExp;
		fc->Cast<FuncCallStat>()->NameExp = prefixExp->Cast<FuncCallExp>()->NameExp;
		fc->Cast<FuncCallStat>()->Args = std::move(prefixExp->Cast<FuncCallExp>()->Args);

		return fc;
	}
	else
	{
		return ParseAssignStat(lexer, prefixExp);
	}
}

StatPtr ParseStat(LexerPtr lexer)
{
	switch (lexer->LookAhead())
	{
		case TOKEN_SEP_SEMI: return ParseEmptyStat(lexer);
		case TOKEN_KW_BREAK: return ParseBreakStat(lexer);
		case TOKEN_SEP_LABEL: return ParseLabelStat(lexer);
		case TOKEN_KW_GOTO: return ParseGotoStat(lexer);
		case TOKEN_KW_DO: return ParseDoStat(lexer);
		case TOKEN_KW_WHILE: return ParseWhileStat(lexer);
		case TOKEN_KW_REPEAT: return ParseRepeatStat(lexer);
		case TOKEN_KW_IF: return ParseIfStat(lexer);
		case TOKEN_KW_FOR: return ParseForStat(lexer);
		case TOKEN_KW_FUNCTION: return ParseFuncDefStat(lexer);
		case TOKEN_KW_LOCAL: return ParseLocalAssignOrFuncDefStat(lexer);
		default: return ParseAssignOrFuncCallStat(lexer);
	}
}

StatArray ParseStats(LexerPtr lexer)
{
	StatArray stats;
	while(!_IsReturnOrBlockEnd(lexer->LookAhead()))
	{
		StatPtr stat = ParseStat(lexer);
		if(!stat->IsA<EmptyStat>())
		{
			stats.push_back(stat);
		}
	}
	return stats;
}

bool _IsReturnOrBlockEnd(int tokenKind)
{
	switch (tokenKind)
	{
		case TOKEN_KW_RETURN:
		case TOKEN_EOF:
		case TOKEN_KW_END:
		case TOKEN_KW_ELSE:
		case TOKEN_KW_ELSEIF:
		case TOKEN_KW_UNTIL:
			return true;
		default:
			return false;
	}
}

ExpArray ParseRetExps(LexerPtr lexer)
{
	ExpArray exps;
	if(lexer->LookAhead() != TOKEN_KW_RETURN)
	{
		return exps;
	}

	lexer->NextToken();
	switch (lexer->LookAhead())
	{
		// next(retstat)
		case TOKEN_EOF:
		case TOKEN_KW_END:
		case TOKEN_KW_ELSE:
		case TOKEN_KW_ELSEIF:
		case TOKEN_KW_UNTIL:
			return exps;
		case TOKEN_SEP_SEMI:
			lexer->NextToken();
			return exps;
		default:
			exps = ParseRetExps(lexer);
			if(lexer->LookAhead() == TOKEN_SEP_SEMI)
			{
				lexer->NextToken();
			}
			return exps;
	}
}

BlockPtr ParseBlock(LexerPtr lexer)
{
	BlockPtr block = BlockPtr(new Block());
	block->Stats = ParseStats(lexer);
	block->RetExps = ParseRetExps(lexer);
	block->LastLine = lexer->Line();
	return block;
}

// exps
ExpPtr ParseNumberExp(LexerPtr lexer)
{	
	TokenResult res = lexer->NextToken();

	auto intPair = ParseInteger(res.token);
	if(std::get<1>(intPair))
	{
		ExpPtr exp;
		exp = Exp::New<IntegerExp>();
		exp->Cast<IntegerExp>()->Line = res.line;
		exp->Cast<IntegerExp>()->Val = std::get<0>(intPair);
		return exp;
	}

	auto floatPair = ParseFloat(res.token);
	if(std::get<1>(floatPair))
	{
		ExpPtr exp;
		exp = Exp::New<FloatExp>();
		exp->Cast<FloatExp>()->Line = res.line;
		exp->Cast<FloatExp>()->Val = std::get<0>(floatPair);
		return exp;
	}

	Error("not a number: %s", res.token.c_str());
	return nullptr;
}

std::pair<ExpPtr, ExpPtr> _ParseField(LexerPtr lexer)
{
	if(lexer->LookAhead() == TOKEN_SEP_LBRACK)
	{
		lexer->NextToken();
		ExpPtr k = ParseExp(lexer);
		lexer->NextTokenKind(TOKEN_SEP_RBRACK);
		lexer->NextTokenKind(TOKEN_OP_ASSIGN);
		ExpPtr v = ParseExp(lexer);
		return std::make_pair(k, v);
	}

	ExpPtr exp = ParseExp(lexer);
	if(exp->IsA<NameExp>())
	{
		NameExp* nameExp = exp->Cast<NameExp>();
		if(lexer->LookAhead() == TOKEN_OP_ASSIGN)
		{
			lexer->NextToken();
			ExpPtr k = Exp::New<StringExp>();
			k->Cast<StringExp>()->Line = nameExp->Line;
			k->Cast<StringExp>()->Val = nameExp->Name;
			ExpPtr v = ParseExp(lexer);
			return std::make_pair(k, v);
		}
	}

	// When the field is not a key = val, then the field is val only.
	// Which means it is an array element.
	return std::make_pair(nullptr, exp);
}

bool _IsFieldSep(int tokenKind)
{
	return tokenKind == TOKEN_SEP_COMMA || tokenKind == TOKEN_SEP_SEMI;
}

std::pair<ExpArray, ExpArray> _ParseFieldList(LexerPtr lexer)
{
	ExpArray ks, vs;
	if(lexer->LookAhead() != TOKEN_SEP_RCURLY)
	{
		auto kvRes = _ParseField(lexer);
		ExpPtr k = std::get<0>(kvRes);
		ExpPtr v = std::get<1>(kvRes);

		ks.emplace_back(k);
		vs.emplace_back(v);

		while(_IsFieldSep(lexer->LookAhead()))
		{
			lexer->NextToken();
			if(lexer->LookAhead() != TOKEN_SEP_RCURLY)
			{
				kvRes = _ParseField(lexer);
				k = std::get<0>(kvRes);
				v = std::get<1>(kvRes);
				ks.emplace_back(k);
				vs.emplace_back(v);
			}
			// The field list ends right now.
			else
			{
				break;
			}
		}
	}

	return std::make_pair(ks, vs);
}

ExpPtr ParseTableConstructorExp(LexerPtr lexer)
{
	int line = lexer->Line();
	lexer->NextTokenKind(TOKEN_SEP_LCURLY);
	auto fieldListRes = _ParseFieldList(lexer);
	ExpArray keyExps = std::move(std::get<0>(fieldListRes));
	ExpArray valExps = std::move(std::get<1>(fieldListRes));
	lexer->NextTokenKind(TOKEN_SEP_RCURLY);
	int lastLine = lexer->Line();

	ExpPtr exp = Exp::New<TableConstructorExp>();
	exp->Cast<TableConstructorExp>()->Line = line;
	exp->Cast<TableConstructorExp>()->LastLine = lastLine;
	exp->Cast<TableConstructorExp>()->KeyExps = std::move(keyExps);
	exp->Cast<TableConstructorExp>()->ValExps = std::move(valExps);
	return exp;
}

std::tuple<StringArray, bool> _ParseParList(LexerPtr lexer)
{
	switch (lexer->LookAhead())
	{
		case TOKEN_SEP_RPAREN:
			return std::make_tuple(StringArray(), false);
		case TOKEN_VARARG:
			lexer->NextToken();
			return std::make_tuple(StringArray(), true);
		default:
			break;
	}

	StringArray names;
	bool isVararg = false;
	names.emplace_back(lexer->NextToken().token);
	while(lexer->LookAhead() == TOKEN_SEP_COMMA)
	{
		lexer->NextToken();
		if(lexer->LookAhead() == TOKEN_IDENTIFIER)
		{
			names.emplace_back(lexer->NextToken().token);
		}
		else
		{
			lexer->NextTokenKind(TOKEN_VARARG);
			isVararg = true;
			break;
		}
	}

	return std::make_tuple(names, isVararg);
}

ExpPtr ParseFuncDefExp(LexerPtr lexer)
{
	int line = lexer->Line();

	lexer->NextTokenKind(TOKEN_SEP_LPAREN);
	auto parRes = _ParseParList(lexer);
	StringArray parList = std::move(std::get<0>(parRes));
	bool isVararg = std::get<1>(parRes);
	lexer->NextTokenKind(TOKEN_SEP_RPAREN);

	BlockPtr block = ParseBlock(lexer);

	int lastLine = lexer->NextTokenKind(TOKEN_KW_END).line;

	ExpPtr exp = Exp::New<FuncDefExp>();
	exp->Cast<FuncDefExp>()->Line = line;
	exp->Cast<FuncDefExp>()->LastLine = lastLine;
	exp->Cast<FuncDefExp>()->ParList = std::move(parList);
	exp->Cast<FuncDefExp>()->IsVararg = isVararg;
	exp->Cast<FuncDefExp>()->Block = block;
	return exp;
}

ExpPtr ParseExp0(LexerPtr lexer)
{
	ExpPtr exp;
	switch (lexer->LookAhead())
	{
		case TOKEN_VARARG:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<VarargExp>();
			exp->Cast<VarargExp>()->Line = res.line;
			break;
		}
		case TOKEN_KW_NIL:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<NilExp>();
			exp->Cast<NilExp>()->Line = res.line;
			break;
		}
		case TOKEN_KW_TRUE:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<TrueExp>();
			exp->Cast<TrueExp>()->Line = res.line;
			break;
		}
		case TOKEN_KW_FALSE:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<FalseExp>();
			exp->Cast<FalseExp>()->Line = res.line;
			break;
		}
		case TOKEN_STRING:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<StringExp>();
			exp->Cast<StringExp>()->Line = res.line;
			exp->Cast<StringExp>()->Val = res.token;
			break;
		}
		case TOKEN_NUMBER:
		{
			exp = ParseNumberExp(lexer);
			break;
		}
		case TOKEN_SEP_LCURLY:
		{
			exp = ParseTableConstructorExp(lexer);
			break;
		}
		case TOKEN_KW_FUNCTION:
		{
			exp = ParseFuncDefExp(lexer);
			break;
		}
		default:
		{
			exp = ParsePrefixExp(lexer);
			break;
		}
	}
	return exp;
}

ExpPtr ParseExp1(LexerPtr lexer)
{
	ExpPtr exp = ParseExp0(lexer);
	while(lexer->LookAhead() == TOKEN_OP_POW)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp0(lexer);
		exp = newExp;
	}
	exp = OptimizePow(exp);
	return exp;
}

ExpPtr ParseExp2(LexerPtr lexer)
{
	ExpPtr exp;
	switch (lexer->LookAhead())
	{
		case TOKEN_OP_UNM:
		case TOKEN_OP_BNOT:
		case TOKEN_OP_LEN:
		case TOKEN_OP_NOT:
		{
			TokenResult res = lexer->NextToken();
			exp = Exp::New<UnopExp>();
			exp->Cast<UnopExp>()->Line = res.line;
			exp->Cast<UnopExp>()->Op = res.kind;
			exp->Cast<UnopExp>()->Exp = ParseExp1(lexer);
			exp = OptimizeUnaryOp(exp);
			break;
		}
		default:
		{
			exp = ParseExp1(lexer);
			break;
		}
	}
	return exp;
}

ExpPtr ParseExp3(LexerPtr lexer)
{
	ExpPtr exp = ParseExp2(lexer);
	while(lexer->LookAhead() == TOKEN_OP_MUL || lexer->LookAhead() == TOKEN_OP_DIV ||
		lexer->LookAhead() == TOKEN_OP_IDIV || lexer->LookAhead() == TOKEN_OP_MOD)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp2(lexer);
		newExp = OptimizeArithBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp4(LexerPtr lexer)
{
	ExpPtr exp = ParseExp3(lexer);
	while(lexer->LookAhead() == TOKEN_OP_ADD || lexer->LookAhead() == TOKEN_OP_SUB)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp3(lexer);
		newExp = OptimizeArithBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp5(LexerPtr lexer)
{
	ExpPtr exp = ParseExp4(lexer);
	if(lexer->LookAhead() != TOKEN_OP_CONCAT)
	{
		return exp;
	}
	ExpArray exps = {exp};
	int line = 0;
	while(lexer->LookAhead() == TOKEN_OP_CONCAT)
	{
		line = lexer->NextToken().line;
		exps.emplace_back(ParseExp4(lexer));
	}
	exp = Exp::New<ConcatExp>();
	exp->Cast<ConcatExp>()->Line = line;
	exp->Cast<ConcatExp>()->Exps = std::move(exps);
	return exp;
}

ExpPtr ParseExp6(LexerPtr lexer)
{
	ExpPtr exp = ParseExp5(lexer);
	while(lexer->LookAhead() == TOKEN_OP_SHL || lexer->LookAhead() == TOKEN_OP_SHR)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp5(lexer);
		newExp = OptimizeBitwiseBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp7(LexerPtr lexer)
{
	ExpPtr exp = ParseExp6(lexer);
	while(lexer->LookAhead() == TOKEN_OP_BAND)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp6(lexer);
		newExp = OptimizeBitwiseBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp8(LexerPtr lexer)
{
	ExpPtr exp = ParseExp7(lexer);
	while(lexer->LookAhead() == TOKEN_OP_BXOR)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp7(lexer);
		newExp = OptimizeBitwiseBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp9(LexerPtr lexer)
{
	ExpPtr exp = ParseExp8(lexer);
	while(lexer->LookAhead() == TOKEN_OP_BOR)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp8(lexer);
		newExp = OptimizeBitwiseBinaryOp(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp10(LexerPtr lexer)
{
	ExpPtr exp = ParseExp9(lexer);
	while(lexer->LookAhead() == TOKEN_OP_LE || lexer->LookAhead() == TOKEN_OP_GE ||
		lexer->LookAhead() == TOKEN_OP_LT || lexer->LookAhead() == TOKEN_OP_GT ||
		lexer->LookAhead() == TOKEN_OP_NE || lexer->LookAhead() == TOKEN_OP_EQ)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp9(lexer);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp11(LexerPtr lexer)
{
	ExpPtr exp = ParseExp10(lexer);
	while(lexer->LookAhead() == TOKEN_OP_AND)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp10(lexer);
		newExp = OptimizeLogicAnd(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp12(LexerPtr lexer)
{
	ExpPtr exp = ParseExp11(lexer);
	while(lexer->LookAhead() == TOKEN_OP_OR)
	{
		TokenResult res = lexer->NextToken();
		ExpPtr newExp = Exp::New<BinopExp>();
		newExp->Cast<BinopExp>()->Line = res.line;
		newExp->Cast<BinopExp>()->Op = res.kind;
		newExp->Cast<BinopExp>()->Exp1 = exp;
		newExp->Cast<BinopExp>()->Exp2 = ParseExp11(lexer);
		newExp = OptimizeLogicOr(newExp);
		exp = newExp;
	}
	return exp;
}

ExpPtr ParseExp(LexerPtr lexer)
{
	return ParseExp12(lexer);
}

ExpArray ParseExpList(LexerPtr lexer)
{
	ExpArray exps;
	exps.push_back(ParseExp(lexer));
	while(lexer->LookAhead() == TOKEN_SEP_COMMA)
	{
		lexer->NextToken();
		exps.push_back(ParseExp(lexer));
	}
	return exps;
}

ExpPtr ParseParensExp(LexerPtr lexer)
{
	lexer->NextTokenKind(TOKEN_SEP_LPAREN);
	ExpPtr exp = ParseExp(lexer);
	lexer->NextTokenKind(TOKEN_SEP_RPAREN);

	// VarargExp and FuncCallExp need to reserve () to get the first argument.
	if(exp->IsA<VarargExp>() || exp->IsA<FuncCallExp>() ||
	// NameExp and TableAccessExp need to reserve () for syntax error
	// if there is an assignment like a,(b),(c[10]) = 1,2,3
	exp->IsA<NameExp>() || exp->IsA<TableAccessExp>())
	{
		ExpPtr newExp = Exp::New<ParensExp>();
		newExp->Cast<ParensExp>()->Exp =exp;
		exp = newExp;
	}

	return exp;
}

ExpPtr _ParseNameExp(LexerPtr lexer)
{
	if(lexer->LookAhead() == TOKEN_SEP_COLON)
	{
		lexer->NextToken();
		TokenKindResult res = lexer->NextIdentifier();
		ExpPtr exp = Exp::New<StringExp>();
		exp->Cast<StringExp>()->Line = res.line;
		exp->Cast<StringExp>()->Val = res.token;
		return exp;
	}
	return nullptr;
}

ExpArray _ParseArgs(LexerPtr lexer)
{
	ExpArray args;
	switch (lexer->LookAhead())
	{
		case TOKEN_SEP_LPAREN:
		{
			lexer->NextToken();
			if(lexer->LookAhead() != TOKEN_SEP_RPAREN)
			{
				args = ParseExpList(lexer);
			}
			lexer->NextTokenKind(TOKEN_SEP_RPAREN);
			break;
		}
		case TOKEN_SEP_LCURLY:
		{
			args.emplace_back(ParseTableConstructorExp(lexer));
			break;
		}
		default:
		{
			TokenKindResult res = lexer->NextTokenKind(TOKEN_STRING);
			ExpPtr exp = Exp::New<StringExp>();
			exp->Cast<StringExp>()->Line = res.line;
			exp->Cast<StringExp>()->Val = res.token;
			args.emplace_back(exp);
			break;
		}
	}
	return args;
}

ExpPtr _FinishFuncCallExp(LexerPtr lexer, ExpPtr prefixExp)
{
	// optional, can be null.
	ExpPtr nameExp = _ParseNameExp(lexer);
	int line = lexer->Line();
	ExpArray args = _ParseArgs(lexer);
	int lastLine = lexer->Line();

	ExpPtr exp = Exp::New<FuncCallExp>();
	exp->Cast<FuncCallExp>()->Line = line;
	exp->Cast<FuncCallExp>()->LastLine = lastLine;
	exp->Cast<FuncCallExp>()->PrefixExp = prefixExp;
	exp->Cast<FuncCallExp>()->NameExp = nameExp;
	exp->Cast<FuncCallExp>()->Args = args;
	return exp;
}

ExpPtr _FinishPrefixExp(LexerPtr lexer, ExpPtr exp)
{
	while(true)
	{
		switch (lexer->LookAhead())
		{
			case TOKEN_SEP_LBRACK:
			{
				lexer->NextToken();
				ExpPtr keyExp = ParseExp(lexer);
				lexer->NextTokenKind(TOKEN_SEP_RBRACK);
				ExpPtr newExp = Exp::New<TableAccessExp>();
				newExp->Cast<TableAccessExp>()->LastLine = lexer->Line();
				newExp->Cast<TableAccessExp>()->PrefixExp = exp;
				newExp->Cast<TableAccessExp>()->KeyExp = keyExp;
				exp = newExp;
				break;
			}
			case TOKEN_SEP_DOT:
			{
				lexer->NextToken();
				TokenKindResult res = lexer->NextIdentifier();

				ExpPtr keyExp = Exp::New<StringExp>();
				keyExp->Cast<StringExp>()->Line = res.line;
				keyExp->Cast<StringExp>()->Val = res.token;

				ExpPtr newExp = Exp::New<TableAccessExp>();
				newExp->Cast<TableAccessExp>()->LastLine = res.line;
				newExp->Cast<TableAccessExp>()->PrefixExp = exp;
				newExp->Cast<TableAccessExp>()->KeyExp = keyExp;
				exp = newExp;
				break;
			}
			case TOKEN_SEP_COLON: // prefixExp:name(args)
			case TOKEN_SEP_LPAREN: // prefixExp(args)
			case TOKEN_SEP_LCURLY: // prefixExp{arg}
			case TOKEN_STRING: // prefix "arg"
			{
				exp = _FinishFuncCallExp(lexer, exp);
				break;
			}
			default:
			{
				return exp;
			}
		}
	}
}

ExpPtr ParsePrefixExp(LexerPtr lexer)
{
	ExpPtr exp;
	if(lexer->LookAhead() == TOKEN_IDENTIFIER)
	{
		TokenKindResult res = lexer->NextIdentifier();
		exp = Exp::New<NameExp>();
		exp->Cast<NameExp>()->Line = res.line;
		exp->Cast<NameExp>()->Name = res.token;
	}
	else
	{
		exp = ParseParensExp(lexer);
	}
	return exp;
}

// optimizer

ExpPtr OptimizeLogicOr(ExpPtr exp)
{
	BinopExp* binExp = exp->Cast<BinopExp>();
	panic_cond(binExp, "expression must be a binary operation");
	if(IsTrue(binExp->Exp1))
	{
		return binExp->Exp1;
	}
	if(IsFalse(binExp->Exp1) && !IsVarargOrFuncCall(binExp->Exp2))
	{
		return binExp->Exp2;
	}
	return exp;
}

ExpPtr OptimizeLogicAnd(ExpPtr exp)
{
	BinopExp* binExp = exp->Cast<BinopExp>();
	panic_cond(binExp, "expression must be a binary operation");
	if(IsFalse(binExp->Exp1))
	{
		return binExp->Exp1;
	}
	if(IsTrue(binExp->Exp1) && !IsVarargOrFuncCall(binExp->Exp2))
	{
		return binExp->Exp2;
	}
	return exp;
}

ExpPtr OptimizeBitwiseBinaryOp(ExpPtr exp)
{
	BinopExp* binExp = exp->Cast<BinopExp>();
	panic_cond(binExp, "expression must be a binary operation");
	if(binExp->Exp1->IsA<IntegerExp>() && binExp->Exp2->IsA<IntegerExp>())
	{
		IntegerExp* i = binExp->Exp1->Cast<IntegerExp>();
		IntegerExp* j = binExp->Exp2->Cast<IntegerExp>();
		switch(binExp->Op)
		{
			case TOKEN_OP_BAND:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = i->Val & j->Val;
				return newExp;
			}
			case TOKEN_OP_BOR:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = i->Val | j->Val;
				return newExp;
			}
			case TOKEN_OP_BXOR:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = i->Val ^ j->Val;
				return newExp;
			}
			case TOKEN_OP_SHL:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = ShiftLeft(i->Val, j->Val);
				return newExp;
			}
			case TOKEN_OP_SHR:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = ShiftRight(i->Val, j->Val);
				return newExp;
			}
		}
	}
	return exp;
}

ExpPtr OptimizeArithBinaryOp(ExpPtr exp)
{
	BinopExp* binExp = exp->Cast<BinopExp>();
	panic_cond(binExp, "expression must be a binary operation");
	if(binExp->Exp1->IsA<IntegerExp>() && binExp->Exp2->IsA<IntegerExp>())
	{
		IntegerExp* x = binExp->Exp1->Cast<IntegerExp>();
		IntegerExp* y = binExp->Exp2->Cast<IntegerExp>();
		// TOKEN_OP_DIV is not processed here,
		// and handed over to the following float logic processing
		switch (binExp->Op)
		{
			case TOKEN_OP_ADD:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = x->Val + y->Val;
				return newExp;
			}
			case TOKEN_OP_SUB:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = x->Val - y->Val;
				return newExp;
			}
			case TOKEN_OP_MUL:
			{
				ExpPtr newExp = Exp::New<IntegerExp>();
				newExp->Cast<IntegerExp>()->Line = binExp->Line;
				newExp->Cast<IntegerExp>()->Val = x->Val * y->Val;
				return newExp;
			}
			case TOKEN_OP_IDIV:
			{
				if(y->Val != 0)
				{
					ExpPtr newExp = Exp::New<IntegerExp>();
					newExp->Cast<IntegerExp>()->Line = binExp->Line;
					newExp->Cast<IntegerExp>()->Val = IFloorDiv(x->Val, y->Val);
					return newExp;
				}
			}
			case TOKEN_OP_MOD:
			{
				if(y->Val != 0)
				{
					ExpPtr newExp = Exp::New<IntegerExp>();
					newExp->Cast<IntegerExp>()->Line = binExp->Line;
					newExp->Cast<IntegerExp>()->Val = IMod(x->Val, y->Val);
					return newExp;
				}
			}
		}
	}

	auto xPair = CastToFloat(binExp->Exp1);
	auto yPair = CastToFloat(binExp->Exp2);
	if(std::get<1>(xPair) && std::get<1>(yPair))
	{
		float x = std::get<0>(xPair);
		float y = std::get<0>(yPair);
		switch (binExp->Op)
		{
			case TOKEN_OP_ADD:
			{
				ExpPtr newExp = Exp::New<FloatExp>();
				newExp->Cast<FloatExp>()->Line = binExp->Line;
				newExp->Cast<FloatExp>()->Val = x + y;
				return newExp;
			}
			case TOKEN_OP_SUB:
			{
				ExpPtr newExp = Exp::New<FloatExp>();
				newExp->Cast<FloatExp>()->Line = binExp->Line;
				newExp->Cast<FloatExp>()->Val = x - y;
				return newExp;
			}
			case TOKEN_OP_MUL:
			{
				ExpPtr newExp = Exp::New<FloatExp>();
				newExp->Cast<FloatExp>()->Line = binExp->Line;
				newExp->Cast<FloatExp>()->Val = x * y;
				return newExp;
			}
			case TOKEN_OP_DIV:
			{
				if(y != 0)
				{
					ExpPtr newExp = Exp::New<FloatExp>();
					newExp->Cast<FloatExp>()->Line = binExp->Line;
					newExp->Cast<FloatExp>()->Val = x / y;
					return newExp;
				}
			}
			case TOKEN_OP_IDIV:
			{
				if(y != 0)
				{
					ExpPtr newExp = Exp::New<FloatExp>();
					newExp->Cast<FloatExp>()->Line = binExp->Line;
					newExp->Cast<FloatExp>()->Val = FFloorDiv(x, y);
					return newExp;
				}
			}
			case TOKEN_OP_MOD:
			{
				if(y != 0)
				{
					ExpPtr newExp = Exp::New<IntegerExp>();
					newExp->Cast<IntegerExp>()->Line = binExp->Line;
					newExp->Cast<IntegerExp>()->Val = FMod(x, y);
					return newExp;
				}
			}
			case TOKEN_OP_POW:
			{
				ExpPtr newExp = Exp::New<FloatExp>();
				newExp->Cast<FloatExp>()->Line = binExp->Line;
				newExp->Cast<FloatExp>()->Val = pow(x, y);
				return newExp;
			}
		}
	}
	return exp;
}

ExpPtr OptimizePow(ExpPtr exp)
{
	if(exp->IsA<BinopExp>())
	{
		BinopExp* binExp = exp->Cast<BinopExp>();
		if(binExp->Op == TOKEN_OP_POW)
		{
			binExp->Exp2 = OptimizePow(binExp->Exp2);
		}
		return OptimizeArithBinaryOp(exp);
	}
	return exp;
}

ExpPtr OptimizeUnaryOp(ExpPtr exp)
{
	UnopExp* unExp = exp->Cast<UnopExp>();
	panic_cond(unExp, "expression must be a unary operation");
	switch(unExp->Op)
	{
		case TOKEN_OP_UNM:
			return OptimizeUnm(exp);
		case TOKEN_OP_NOT:
			return OptimizeNot(exp);
		case TOKEN_OP_BNOT:
			return OptimizeBnot(exp);
		default:
			return exp;
	}
}

ExpPtr OptimizeUnm(ExpPtr exp)
{
	UnopExp* unExp = exp->Cast<UnopExp>();
	panic_cond(unExp, "expression must be a unary operation");
	ExpPtr soulExp = unExp->Exp;
	if(soulExp->IsA<IntegerExp>())
	{
		IntegerExp* x = soulExp->Cast<IntegerExp>();
		x->Val = -x->Val;
		return soulExp;
	}
	if(soulExp->IsA<FloatExp>())
	{
		FloatExp* x = soulExp->Cast<FloatExp>();
		// There is a difference between -0 and +0.
		if(x->Val != 0)
		{
			x->Val = -x->Val;
			return soulExp;
		}
	}
	return exp;
}

ExpPtr OptimizeNot(ExpPtr exp)
{
	UnopExp* unExp = exp->Cast<UnopExp>();
	panic_cond(unExp, "expression must be a unary operation");
	ExpPtr soulExp = unExp->Exp;
	if(soulExp->IsA<NilExp>() || soulExp->IsA<FalseExp>())
	{
		ExpPtr newExp = Exp::New<TrueExp>();
		newExp->Cast<TrueExp>()->Line = unExp->Line;
		return newExp;
	}
	if(soulExp->IsA<TrueExp>() || soulExp->IsA<IntegerExp>()
	|| soulExp->IsA<FloatExp>() || soulExp->IsA<StringExp>())
	{
		ExpPtr newExp = Exp::New<FalseExp>();
		newExp->Cast<FalseExp>()->Line = unExp->Line;
		return newExp;
	}
	return exp;
}

ExpPtr OptimizeBnot(ExpPtr exp)
{
	UnopExp* unExp = exp->Cast<UnopExp>();
	panic_cond(unExp, "expression must be a unary operation");
	ExpPtr soulExp = unExp->Exp;
	if(soulExp->IsA<IntegerExp>())
	{
		IntegerExp* x = soulExp->Cast<IntegerExp>();
		x->Val = ~x->Val;
		return soulExp;
	}
	if(soulExp->IsA<FloatExp>())
	{
		FloatExp* x = soulExp->Cast<FloatExp>();
		auto toInt = FloatToInteger(x->Val);
		if(std::get<1>(toInt))
		{
			Int64 val = std::get<0>(toInt);
			ExpPtr newExp = Exp::New<IntegerExp>();
			newExp->Cast<IntegerExp>()->Line = x->Line;
			newExp->Cast<IntegerExp>()->Val = ~val;
			return newExp;
		}
	}
	return exp;
}

bool IsFalse(ExpPtr exp)
{
	if(exp->IsA<FalseExp>() || exp->IsA<NilExp>())
		return true;
	return false;
}

bool IsTrue(ExpPtr exp)
{
	if(exp->IsA<TrueExp>() || exp->IsA<IntegerExp>()
		|| exp->IsA<FloatExp>() || exp->IsA<StringExp>())
		return true;
	return false;
}

bool IsVarargOrFuncCall(ExpPtr exp)
{
	if(exp->IsA<VarargExp>() || exp->IsA<FuncCallExp>())
		return true;
	return false;
}

std::tuple<Int64, bool> CastToInt(ExpPtr exp)
{
	if(exp->IsA<IntegerExp>())
	{
		return std::make_pair(
			exp->Cast<IntegerExp>()->Val,
			true);
	}
	if(exp->IsA<FloatExp>())
	{
		return FloatToInteger(exp->Cast<FloatExp>()->Val);
	}
	return std::make_pair(0, false);
}

std::tuple<Float64, bool> CastToFloat(ExpPtr exp)
{
	if(exp->IsA<IntegerExp>())
	{
		return std::make_pair(
			(Float64)exp->Cast<IntegerExp>()->Val,
			true);
	}
	if(exp->IsA<FloatExp>())
	{
		return std::make_pair(
			exp->Cast<FloatExp>()->Val,
			true);
	}
	return std::make_pair(0, false);
}