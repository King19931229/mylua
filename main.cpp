#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "state/lua_state.h"
#include "number/parser.h"
#include "state/api_arith.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "compiler/dumper/dumper.h"

String KindToCategory(int kind)
{
	if(kind < TOKEN_SEP_SEMI)
		return "other";
	if(kind <= TOKEN_SEP_RCURLY)
		return "separator";
	if(kind <= TOKEN_OP_NOT)
		return "operator";
	if(kind <= TOKEN_KW_WHILE)
		return "keyword";
	if(kind == TOKEN_IDENTIFIER)
		return "identifier";
	if(kind == TOKEN_NUMBER)
		return "number";
	if(kind == TOKEN_STRING)
		return "string";
	return "other";	
}

void testLexer(const String& chunk, const String& chunkName)
{
	LexerPtr lexer = NewLexer(chunk, chunkName);
	while(true)
	{
		TokenResult res = lexer->NextToken();
		String msg = Format::FormatString("[%2d] [%-10s] %s", res.line,
			KindToCategory(res.kind).c_str(),
			res.token.c_str());
		printf("%s\n", msg.c_str());
		if(res.kind == TOKEN_EOF)
			break;
	}
}

void testParser(const String& chunk, const String& chunkName)
{
	BlockPtr ast = Parse(chunk, chunkName);
	FILE* fp = fopen("parser_tree.txt", "wb");
	DumpBlock(ast, 0, fp);
	fclose(fp);
}

int main()
{
#if 0
	FILE* f = fopen("hello.lua", "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<Byte> buffer;
		buffer.resize(size);

		fread(buffer.data(), 1, size, f);
		fclose(f);
		f = NULL;

		String chunk;
		chunk.reserve(buffer.size());
		for(const Byte& c : buffer)
		{
			chunk += c;
		}

		//testLexer(chunk, "main");
		testParser(chunk, "main");
	}
#else
	//FILE* f = fopen("C:/LearnCompiler/lua-5.3.6/src/hello.luac", "rb");
	FILE* f = fopen("hello.lua", "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<Byte> buffer;
		buffer.resize(size);
		fread(buffer.data(), 1, size, f);
		fclose(f);
		f = NULL;

		LuaStatePtr state = NewLuaState();
		state->OpenLibs();
		state->Load(buffer, "chunk", "b");
		state->Call(0, 0);
	}
#endif
	system("pause");
}