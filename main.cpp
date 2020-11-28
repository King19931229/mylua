#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "state/lua_state.h"
#include "number/parser.h"
#include "state/api_arith.h"

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
			printf("%s", TypeName(ls->Type(i)).c_str());
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

int main()
{
	FILE* f = fopen("C:/LearnCompiler/lua-5.3.6/src/hello.luac", "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<unsigned char> buffer;
		buffer.resize(size + 1);
		buffer[size] = 0;

		fread(buffer.data(), 1, size, f);
		fclose(f);
		f = NULL;

		LuaStatePtr state = NewLuaState();
		state->Register("print", print);
		state->Register("getmetatable", getMetatable);
		state->Register("setmetatable", setMetatable);
		state->Load(buffer, "chunk", "b");
		state->Call(0, 0);
	}
	system("pause");
}
