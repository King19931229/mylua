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
	// todo
	return ParseEmptyStat(lexer);
}

StatPtr ParseForStat(LexerPtr lexer)
{
	// todo
	return ParseEmptyStat(lexer);
}

StatPtr ParseFuncDefStat(LexerPtr lexer)
{
	// todo
	return ParseEmptyStat(lexer);
}

StatPtr ParseLocalAassignOrFuncDefStat(LexerPtr lexer)
{
	// todo
	return ParseEmptyStat(lexer);
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