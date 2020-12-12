#pragma once
#include "compiler/ast/block.h"
#include "compiler/ast/exp.h"
#include "compiler/ast/stat.h"
#include "compiler/parser/parser_block.h"
#include "compiler/lexer/lexer.h"

inline BlockPtr Parse(const String& chunk, const String& chunkName)
{
	LexerPtr lexer = NewLexer(chunk, chunkName);
	BlockPtr block = ParseBlock(lexer);
	lexer->NextTokenKind(TOKEN_EOF);
	return block;
}