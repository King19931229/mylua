#pragma once
#include "ast/exp.h"
#include "lexer/lexer.h"

ExpPtr ParseExp(LexerPtr lexer);
ExpArray ParseExpList(LexerPtr lexer);
ExpPtr ParsePrefixExp(LexerPtr lexer);