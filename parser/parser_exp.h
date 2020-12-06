#pragma once
#include "ast/exp.h"
#include "lexer/lexer.h"

ExpPtr ParseExp(LexerPtr Lexer);
ExpArray ParseExpList(LexerPtr Lexer);