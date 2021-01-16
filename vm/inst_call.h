#pragma once
#include "state/lua_state.h"

struct __call_insts__
{
	static void closure(Instruction i, LuaVM* vm)
	{
		auto abx = i.ABx();
		int a = std::get<0>(abx) + 1;
		int bx = std::get<1>(abx);
		vm->LoadProto(bx);
		vm->Replace(a);
	}

	// Push function a and the first half of the parameters
	// (if is called by _return 'a' is just the first parameter)
	// and the return value of function x (the second half of the parameters)
	// to the top of the stack
	static void _fixStack(int a, LuaVM* vm)
	{
		int x = (int)vm->ToInteger(-1);
		vm->Pop(1);

		vm->CheckStack(x - a);
		for(int i = a; i < x; ++i)
			vm->PushValue(i);
		// vm->RegisterCount() + 1 point to the return value of x
		vm->Rotate(vm->RegisterCount() + 1, x - a);
	}

	static int _pushFuncAndArgs(int a, int b, LuaVM* vm)
	{
		// b - 1 args
		if(b >= 1)
		{
			vm->CheckStack(b);
			for(int i = a; i < a + b; ++i)
			{
				vm->PushValue(i);
			}
			return b - 1;
		}
		else
		{
			_fixStack(a, vm);
			// [top - (r + 1) + 1] - 1
			return vm->GetTop() - vm->RegisterCount() - 1;
		}
	}

	// Push the return value to position a according to c
	static void _popResults(int a, int c, LuaVM* vm)
	{
		if(c == 1)
		{
			// no results
		}
		else if(c > 1)
		{
			// Push c - 1 return value from the top of the stack to a
			for(int i = a + c - 2; i >= a; --i)
			{
				vm->Replace(i);
			}
		}
		else // c == 0
		{
			// Push the position of the called function on the stack
			vm->CheckStack(1);
			vm->PushInteger(a);
		}
	}

	static void call(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		int nArgs = _pushFuncAndArgs(a, b, vm);
		vm->Call(nArgs, c - 1);
		_popResults(a, c, vm);
	}

	static void _return(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		// a the position of the return value
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_);

		if(b == 1)
		{
			// no return values
		}
		else if(b > 1)
		{
			vm->CheckStack(b - 1);
			for(int i = a; i <= a + b - 2; ++i)
			{
				vm->PushValue(i);
			}
		}
		else
		{
			_fixStack(a, vm);
		}
	}

	static void vararg(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_);

		if(b != 1)
		{
			vm->LoadVararg(b - 1);
			_popResults(a, b, vm);
		}
	}

	static void tailCall(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_);
		int c = 0;

		/*
		int nArgs = _pushFuncAndArgs(a, b, vm);
		vm->Call(nArgs, c - 1);
		_popResults(a, c, vm);
		*/

		panic_cond(b >= 1, "b must >= 1");
		vm->TailCall(a, b - 1);
		_popResults(a, c, vm);
	}

	static void self(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		int c = std::get<2>(abc);

		vm->Copy(b, a + 1);
		vm->GetRK(c);
		vm->GetTable(b);
		vm->Replace(a);
	}

	static void tForCall(Instruction i, LuaVM* vm)
	{
		auto a_c = i.ABC();
		int a = std::get<0>(a_c) + 1;
		int c = std::get<2>(a_c);

		_pushFuncAndArgs(a, 3, vm);
		vm->Call(2, c);
		_popResults(a + 3, c + 1, vm);
	}

	static void tForLoop(Instruction i, LuaVM* vm)
	{
		auto asBx = i.ABsBx();
		int a = std::get<0>(asBx) + 1;
		int sBx = std::get<1>(asBx);
		if(!vm->IsNil(a + 1))
		{
			vm->Copy(a + 1, a);
			vm->AddPC(sBx);
		}
	}
};