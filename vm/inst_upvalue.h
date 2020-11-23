#pragma once
#include "state/lua_state.h"

struct __upvalue_inst__
{
	static void getTabUp(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		int c = std::get<2>(abc);

		vm->GetRK(c);
		vm->GetTable(LuaUpvalueIndex(b));
		vm->Replace(a);
	}

	static void setTabUp(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);

		vm->GetRK(b);
		vm->GetRK(c);
		vm->SetTable(LuaUpvalueIndex(a));
	}

	static void getUpVal(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_) + 1;
		vm->Copy(LuaUpvalueIndex(b), a);
	}

	static void setUpVal(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_) + 1;
		vm->Copy(a, LuaUpvalueIndex(b));
	}
};