#pragma once
#include "state/lua_state.h"

struct __upvalue_inst__
{
	static void getTabUp(Instruction i, LuaVM* vm)
	{
		auto a_c = i.ABC();
		int a = std::get<0>(a_c) + 1;
		int c = std::get<2>(a_c);

		vm->PushGlobalTable();
		vm->GetRK(c);
		vm->GetTable(-2);
		vm->Replace(a);
	}
};