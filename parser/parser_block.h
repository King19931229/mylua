#pragma once
#include "ast/block.h"
#include "lexer/lexer.h"

// ';'
StatPtr ParseEmptyStat(LexerPtr lexer);
// break
StatPtr ParseBreakStat(LexerPtr lexer);
// '::' Name '::'
StatPtr ParseLabelStat(LexerPtr lexer);
// goto Name
StatPtr ParseGotoStat(LexerPtr lexer);
// do block end
StatPtr ParseDoStat(LexerPtr lexer);
// while exp do block end
StatPtr ParseWhileStat(LexerPtr lexer);
// repeat block until exp
StatPtr ParseRepeatStat(LexerPtr lexer);
// if exp then block {elseif exp then block} [else block] end
StatPtr ParseIfStat(LexerPtr lexer);
// for Name '=' exp ',' exp [',' exp] do block end
// for namelist in explist do block end
StatPtr ParseForStat(LexerPtr lexer);
// function funcname funcbody
StatPtr ParseFuncDefStat(LexerPtr lexer);
// loval function Name funcbody
// local namelist ['=' explist]
StatPtr ParseLocalAassignOrFuncDefStat(LexerPtr lexer);
// varlist '=' explist
// functioncall
StatPtr ParseAssignOrFuncCallStat(LexerPtr lexer);

// stat
StatPtr ParseStat(LexerPtr lexer);

// {stat}
StatArray ParseStats(LexerPtr lexer);

// next(stat)
bool _IsReturnOrBlockEnd(int tokenKind);

// retstat ::= return [explist][';']
ExpArray ParseRetExps(LexerPtr lexer);

// block ::= {stat}[retstat]
BlockPtr ParseBlock(LexerPtr lexer);