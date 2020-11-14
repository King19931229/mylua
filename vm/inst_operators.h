#pragma once
#include "state/lua_state.h"

inline void _BinaryArith(Instruction i, LuaVM* vm, ArithOp op)
{
	auto abc = i.ABC();
	int a = std::get<0>(abc) + 1;
	int b = std::get<1>(abc);
	int c = std::get<2>(abc);
	vm->GetRK(b);
	vm->GetRK(c);
	vm->Arith(op);
	vm->Replace(a);
}

inline void _UnaryArith(Instruction i, LuaVM* vm, ArithOp op)
{
	auto ab_ = i.ABC();
	int a = std::get<0>(ab_) + 1;
	int b = std::get<1>(ab_) + 1;
	vm->GetRK(b);
	vm->Arith(op);
	vm->Replace(a);
}

struct __binary_insts__
{
	static void add(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPADD); }
	static void sub(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPSUB); }
	static void mul(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPMUL); }
	static void mod(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPMOD); }
	static void pow(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPPOW); }
	static void div(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPDIV); }
	static void idiv(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPIDIV); }
	static void band(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPBAND); }
	static void bor(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPBOR); }
	static void bxor(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPBXOR); }
	static void shl(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPSHL); }
	static void shr(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPSHR); }
	static void unm(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPUNM); }
	static void bnot(Instruction i, LuaVM* vm) { _BinaryArith(i, vm, LUA_OPBNOT); }
};

struct __str_insts__
{
	static void length(Instruction i, LuaVM* vm)
	{
		auto ab_ = i.ABC();
		int a = std::get<0>(ab_) + 1;
		int b = std::get<1>(ab_) + 1;
		vm->Len(b);
		vm->Replace(a); 
	}

	static void cat(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		int c = std::get<2>(abc) + 1;
		int n = c - b + 1;
		vm->CheckStack(n);
		for(int i = b; i <= c; ++i)
			// Don't use GetRK. GetRK will add 1 for register
			vm->PushValue(i);
		vm->Concat(n);
		vm->Replace(a);
	}
};

inline void _Compare(Instruction i, LuaVM* vm, CompareOp op)
{
	auto abc = i.ABC();
	int a = std::get<0>(abc);
	int b = std::get<1>(abc);
	int c = std::get<2>(abc);
	vm->GetRK(b);
	vm->GetRK(c);
	if(vm->Compare(-2, -1, op) != (a != 0))
		vm->AddPC(1);
	vm->Pop(2);
}

struct __compare_insts__
{
	static void eq(Instruction i, LuaVM* vm) { _Compare(i, vm, LUA_OPEQ); }
	static void lt(Instruction i, LuaVM* vm) { _Compare(i, vm, LUA_OPLT); }
	static void le(Instruction i, LuaVM* vm) { _Compare(i, vm, LUA_OPLE); }
};

struct __other_insts__
{
	static void _not(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		vm->PushBoolean(!vm->ToBoolean(b));
		vm->Replace(a);
	}

	static void testSet(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int b = std::get<1>(abc) + 1;
		int c = std::get<1>(abc);
		if(vm->ToBoolean(b) == (c != 0))
		{
			vm->Copy(b, a);
		}
		else
		{
			vm->AddPC(1);
		}
	}

	static void test(Instruction i, LuaVM* vm)
	{
		auto abc = i.ABC();
		int a = std::get<0>(abc) + 1;
		int c = std::get<1>(abc);
		if(vm->ToBoolean(a) != (c != 0))
		{
			vm->AddPC(1);
		}
	}

	static void forPrep(Instruction i, LuaVM* vm)
	{
		auto asbx = i.ABsBx();
		int a = std::get<0>(asbx) + 1;
		int sbx = std::get<1>(asbx);

		// R(A) -= R(A + 2)
		vm->PushValue(a);
		vm->PushValue(a + 2);
		vm->Arith(LUA_OPSUB);
		vm->Replace(a);
		// pc += sbx
		vm->AddPC(sbx);
	}

	static void forLoop(Instruction i, LuaVM* vm)
	{
		auto asbx = i.ABsBx();
		int a = std::get<0>(asbx) + 1;
		int sbx = std::get<1>(asbx);

		// R(A) += R(A + 2)
		vm->PushValue(a);
		vm->PushValue(a + 2);
		vm->Arith(LUA_OPADD);
		vm->Replace(a);

		// R(A) <?= R(A + 1)
		bool isPositiveStep = vm->ToNumber(a + 2) >= 0;
		if((isPositiveStep && vm->Compare(a, a + 1, LUA_OPLE)) ||
			(!isPositiveStep && vm->Compare(a + 1, a, LUA_OPLE)))
		{
			// pc += sbx
			vm->AddPC(sbx);
			// R(A + 3) = R(A)
			vm->Copy(a, a + 3);
		}
	}
};