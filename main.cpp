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
	char a = -1;
	printf("%x\n", a);
	LuaState state = NewLuaState(20, nullptr);
	state.PushInteger(1);
	state.PushString("2.0");
	state.PushString("3.0");
	state.PushNumber(4.0);
	PrintStack(state);

	state.Arith(LUA_OPADD); PrintStack(state);
	state.Arith(LUA_OPBNOT); PrintStack(state);
	state.Len(2); PrintStack(state);
	state.Concat(3); PrintStack(state);
	state.PushBoolean(state.Compare(1, 2, LUA_OPEQ));
	PrintStack(state);
#if 0
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
#endif
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
