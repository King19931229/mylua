#include "parser/parser_block.h"
#include "parser/parser_exp.h"
#include "ast/stat.h"

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

StatPtr ParseLocalAassignOrFuncDefStat(LexerPtr lexer)
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

StatPtr ParseAssignOrFuncCallStat(LexerPtr lexer)
{
	// todo
	return ParseEmptyStat(lexer);
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
		case TOKEN_KW_LOCAL: return ParseLocalAassignOrFuncDefStat(lexer);
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
ExpPtr ParseExp(LexerPtr lexer)
{
	return Exp::New<TrueExp>();
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