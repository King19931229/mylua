#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"
#include "state/lua_state.h"
#include "number/parser.h"

int main()
{
	{
		std::tuple<Float64, bool> pair;
		pair = ParseFloat("333.33232");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("0.332");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("0001.332");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("0001.");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("1089891.f");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("101.0ffff");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseFloat("100");
		printf("%f %d\n", std::get<0>(pair), std::get<1>(pair));
	}
	{
		std::tuple<Int64, bool> pair;
		pair = ParseInteger("00333");
		printf("%I64d %d\n", std::get<0>(pair), std::get<1>(pair));
		pair = ParseInteger("3332312");
		printf("%I64d %d\n", std::get<0>(pair), std::get<1>(pair));
	}
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
