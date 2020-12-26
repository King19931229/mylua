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

int print(LuaState* ls)
{
	int nArgs = ls->GetTop();
	for(int i = 1; i <= nArgs; ++i)
	{
		if(ls->IsBoolean(i))
		{
			printf("%s", Format::FromBool(ls->ToBoolean(i)).c_str());
		}
		else if(ls->IsString(i))
		{
			printf("%s", ls->ToString(i).c_str());
		}
		else
		{
			printf("%s", ls->TypeName(ls->Type(i)).c_str());
		}
		if(i < nArgs)
		{
			printf("\t");
		}
	}

	puts("");
	return 0;
}

int getMetatable(LuaState* ls)
{
	if(!ls->GetMetatable(1))
	{
		ls->PushNil();
	}
	return 1;
}

int setMetatable(LuaState* ls)
{
	ls->SetMetatable(1);
	// return the value itself
	return 1;
}

int next(LuaState* ls)
{
	ls->SetTop(2);
	if(ls->Next(1))
	{
		return 2;
	}
	else
	{
		ls->PushNil();
		return 1;
	}
}

int pairs(LuaState* ls)
{
	ls->PushCFunction(next);
	ls->PushValue(1);
	ls->PushNil();
	return 3;
}

int _ipairsAux(LuaState* ls)
{
	Int64 i = ls->ToInteger(2) + 1;
	ls->PushInteger(i);
	if(ls->GetI(1, i) == LUA_TNIL)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

int ipairs(LuaState* ls)
{
	ls->PushCFunction(_ipairsAux);
	ls->PushValue(1);
	ls->PushNil();
	return 3;
}

int error(LuaState* ls)
{
	return ls->Error();
}

int pcall(LuaState* ls)
{
	int nArgs = ls->GetTop() - 1;
	int status = ls->PCall(nArgs, -1,  0);
	ls->PushBoolean(status == LUA_OK);
	ls->Insert(1);
	return ls->GetTop();
}

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
	FILE* f = fopen("C:/LearnCompiler/lua-5.3.6/src/hello.lua", "rb");
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
	FILE* f = fopen("C:/LearnCompiler/lua-5.3.6/src/hello.lua", "rb");
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
		state->Register("print", print);
		state->Register("getmetatable", getMetatable);
		state->Register("setmetatable", setMetatable);
		state->Register("next", next);
		state->Register("pairs", pairs);
		state->Register("ipairs", ipairs);
		state->Register("error", error);
		state->Register("pcall", pcall);
		// state->OpenLibs();
		state->Load(buffer, "chunk", "b");
		state->Call(0, 0);
	}
#endif
	system("pause");
}
