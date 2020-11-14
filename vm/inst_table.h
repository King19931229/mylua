#pragma once
#include "state/lua_state.h"

struct __table_insts__
{
	static void newTable(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		vm->CreateTable(b, c);
		vm->Replace(a);
	}

	static void getTable(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		int c = std::get<2>(abc);
		vm->GetRK(c);
		vm->GetTable(b);
		vm->Replace(a);
	}

	static void setTable(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		vm->GetRK(b);
		vm->GetRK(c);
		vm->SetTable(a);
	}

	static void setList(Instruction i, LuaVM* vm)
	{
		static const int LFIELDS_PER_FLUSH = 50;

		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		if(c > 0)
		{
			c -= 1;
		}
		else
		{
			c = Instruction(vm->Fetch()).Ax();
		}

		Int64 idx = c * LFIELDS_PER_FLUSH;
		for(int i = 1; i <= b; ++i)
		{
			vm->PushValue(a + i);
			++idx;
			vm->SetI(a, idx);
		}
	}
};