#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "state/lua_state.h"

int main()
{
	LuaState state = NewLuaState();
	state.PushBoolean(true); PrintStack(state);
	state.PushInteger(10); PrintStack(state);
	state.PushNil(); PrintStack(state);
	state.PushString("hello"); PrintStack(state);
	state.PushValue(-4); PrintStack(state);
	state.Replace(3); PrintStack(state);
	state.SetTop(6); PrintStack(state);
	state.Remove(-3); PrintStack(state);
	state.SetTop(-5); PrintStack(state);
#if 0
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

		Prototype* type = Undump(buffer);
		type->List();
	}
#endif
}
