#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "state/lua_state.h"
#include "number/parser.h"
#include "state/api_arith.h"

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
		state->Load(buffer, "", "b");
		state->Call(0, 0);
	}
}
