#pragma once
#include "state/lua_state.h"

struct __inst_misc__
{
	static void move(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		vm->Copy(b, a);
	}

	static void jmp(Instruction i, LuaVM* vm)
	{
		auto asbx = i.ABsBx();
		int a = std::get<0>(asbx);
		int sbx = std::get<1>(asbx);
		vm->AddPC(sbx);
		// about upvalue
		if(a != 0)
			panic("todo");
	}

	static void loadNil(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		vm->PushNil();
		for(int i = a; i <= a + b; ++i)
			vm->Copy(-1, i);
		vm->Pop(1);
	}

	static void loadBool(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		vm->PushBoolean(b != 0);
		vm->Replace(a);
		if(c)
			vm->AddPC(1);
	}

	static void loadK(Instruction i, LuaVM* vm)
	{
		auto abx = i.ABx();
		int a = std::get<0>(abx) + 1;
		int bx = std::get<1>(abx);
		vm->GetConst(bx);
		vm->Replace(a);
	}

	static void loadKx(Instruction i, LuaVM* vm)
	{
		auto a_ = i.ABx();
		int a = std::get<0>(a_) + 1;
		int ax = Instruction{vm->Fetch()}.Ax();
		vm->GetConst(ax);
		vm->Replace(a);
	}
};