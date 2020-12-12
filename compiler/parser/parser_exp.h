#pragma once
#include "compiler/ast/exp.h"
#include "compiler/lexer/lexer.h"

ExpPtr ParseExp(LexerPtr lexer);
ExpArray ParseExpList(LexerPtr lexer);
ExpPtr ParsePrefixExp(LexerPtr lexer);