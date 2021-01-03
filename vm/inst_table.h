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
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);

		bool bIsZero = b == 0;
		if(bIsZero)
		{
			b = (int)vm->ToInteger(-1) - a - 1;
			vm->Pop(1); 
		}

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

		if(bIsZero)
		{
			for(int j = vm->RegisterCount() + 1; j <= vm->GetTop(); ++j)
			{
				++idx;
				vm->PushValue(j);
				vm->SetI(a, idx);
			}
			vm->SetTop(vm->RegisterCount());
		}
	}
};