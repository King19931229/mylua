#include "state/lua_value.h"
#include "state/lua_table.h"
#include "state/api_arith.h"
#include "vm/inst_misc.h"
#include "vm/inst_operators.h"
#include "vm/inst_table.h"
#include "vm/inst_call.h"
#include "vm/inst_upvalue.h"
#include "vm/opcodes.h"

const OpCode opcodes[47] =
{
#define MAKE_OP_CODE(T, A, B, C, mode, name, action) OpCode{T, A, B, C, mode, #name, action}
	MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, MOVE, __inst_misc__::move)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgN, iABx, LOADK, __inst_misc__::loadK)
	,MAKE_OP_CODE(0, 1, OpArgN, OpArgN, iABx, LOADKX, __inst_misc__::loadKx)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, LOADBOOL, __inst_misc__::loadBool)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, LOADNIL, __inst_misc__::loadNil)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, GETUPVAL, __upvalue_inst__::getUpVal)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgK, iABC, GETTABUP, __upvalue_inst__::getTabUp)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgK, iABC, GETTABLE, __table_insts__::getTable)
	,MAKE_OP_CODE(0, 0, OpArgK, OpArgK, iABC, SETTABUP, __upvalue_inst__::setTabUp)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgN, iABC, SETUPVAL, __upvalue_inst__::setUpVal)
	,MAKE_OP_CODE(0, 0, OpArgK, OpArgK, iABC, SETTABLE, __table_insts__::setTable)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, NEWTABLE, __table_insts__::newTable)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgK, iABC, SELF, __call_insts__::self)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, ADD, __binary_insts__::add)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SUB, __binary_insts__::sub)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, MUL, __binary_insts__::mul)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, MOD, __binary_insts__::mod)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, POW, __binary_insts__::pow)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, DIV, __binary_insts__::div)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, IDIV, __binary_insts__::idiv)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BAND, __binary_insts__::band)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BOR, __binary_insts__::bor)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BXOR, __binary_insts__::bxor)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SHL, __binary_insts__::shl)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SHR, __binary_insts__::shr)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, UNM, __binary_insts__::unm)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, BNOT, __binary_insts__::bnot)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, NOT, __other_insts__::_not)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, LEN, __str_insts__::length)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgR, iABC, CONCAT, __str_insts__::cat)
	,MAKE_OP_CODE(0, 0, OpArgR, OpArgN, iAsBx, JMP, __inst_misc__::jmp)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, EQ, __compare_insts__::eq)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, LT, __compare_insts__::lt)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, LE, __compare_insts__::le)
	,MAKE_OP_CODE(1, 0, OpArgN, OpArgU, iABC, TEST, __other_insts__::test)
	,MAKE_OP_CODE(1, 1, OpArgR, OpArgU, iABC, TESTSET, __other_insts__::testSet)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, CALL, __call_insts__::call)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, TAILCALL, __call_insts__::tailCall)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgN, iABC, RETURN, __call_insts__::_return)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, FORLOOP, __other_insts__::forLoop)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, FORPREP, __other_insts__::forPrep)
	,MAKE_OP_CODE(0, 0, OpArgN, OpArgU, iABC, TFORCALL, nullptr)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, TFORLOOP, nullptr)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgU, iABC, SETLIST, __table_insts__::setList)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABx, CLOSURE, __call_insts__::closure)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, VARARG, __call_insts__::vararg)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgU, iAx, EXTRAARG, nullptr)
#undef MAKE_OP_CODE
};

struct __funcs__
{
	static Int64 iadd(Int64 a, Int64 b) { return a + b; }
	static Float64 fadd(Float64 a, Float64 b) { return a + b; }
	static Int64 isub(Int64 a, Int64 b) { return a - b; }
	static Float64 fsub(Float64 a, Float64 b) { return a - b; }
	static Int64 imul(Int64 a, Int64 b) { return a * b; }
	static Float64 fmul(Float64 a, Float64 b) { return a * b; }
	static Int64 imod(Int64 a, Int64 b) { return IMod(a, b); }
	static Float64 fmod(Float64 a, Float64 b) { return FMod(a, b); }
	static Float64 pow(Float64 a, Float64 b) { return ::pow(a, b); }
	static Float64 div(Float64 a, Float64 b) { return a / b; }
	static Int64 iidiv(Int64 a, Int64 b) { return IFloorDiv(a, b); }
	static Float64 fidiv(Float64 a, Float64 b) { return FFloorDiv(a, b); }
	static Int64 band(Int64 a, Int64 b) { return a & b; }
	static Int64 bor(Int64 a, Int64 b) { return a | b; }
	static Int64 bxor(Int64 a, Int64 b) { return a ^ b; }
	static Int64 shl(Int64 a, Int64 b) { return ShiftLeft(a, b); }
	static Int64 shr(Int64 a, Int64 b) { return ShiftRight(a, b); }
	static Int64 iunm(Int64 a, Int64) { return -a; }
	static Float64 funm(Float64 a, Float64) { return -a; }
	static Int64 bnot(Int64 a, Int64) { return ~a; }
};

const Operator operators[14] =
{
	Operator{"__add", __funcs__::iadd, __funcs__::fadd},
	Operator{"__sub", __funcs__::isub, __funcs__::fsub},
	Operator{"__mul",__funcs__::imul, __funcs__::fmul},
	Operator{"__mod", __funcs__::imod, __funcs__::fmod},
	Operator{"__pow", nullptr, __funcs__::pow},
	Operator{"__div", nullptr, __funcs__::div},
	Operator{"__idiv", __funcs__::iidiv, __funcs__::fidiv},
	Operator{"__band", __funcs__::band, nullptr},
	Operator{"__bor", __funcs__::bor, nullptr},
	Operator{"__bxor", __funcs__::bxor, nullptr},
	Operator{"__shl", __funcs__::shl, nullptr},
	Operator{"__shr", __funcs__::shr, nullptr},
	Operator{"__unm", __funcs__::iunm, __funcs__::funm},
	Operator{"__bnot", __funcs__::bnot, nullptr},
};

void Instruction::Execute(LuaVM *vm)
{
	OpCode::Func action = opcodes[Opcode()].action;
	if(action)
		action(*this, vm);
	else
		warning(OpName().c_str());
}

void SetMetatable(LuaValue& val, LuaTablePtr mt, LuaState* ls)
{
	if(val.IsTable())
	{
		DEBUG_PRINT("SetMetatable: 0x%x 0x%x\n", (size_t)val.table.get(), (size_t)mt.get());
		val.table->metatable = mt;
		return;
	}
	String key = Format::FormatString("_MT%d", val.tag);
	ls->registry->Put(LuaValue(key), LuaValue(mt));
}

LuaTablePtr GetMetatable(const LuaValue& val, const LuaState* ls)
{
	if(val.IsTable())
	{
		DEBUG_PRINT("GetMetatable: 0x%x\n", (size_t)val.table.get());
		return val.table->metatable;
	}
	String key = Format::FormatString("_MT%d", val.tag);
	LuaValuePtr mt = ls->registry->Get(LuaValue(key));
	if(mt && mt->IsTable())
	{
		return mt->table;
	}
	return nullptr;
}

std::tuple<LuaValue, bool> CallMetamethod(const LuaValue& a, const LuaValue& b,
		const String& mmName, LuaState* ls)
{
	LuaValue mm;
	mm = GetMetafield(a, mmName, ls);
	if(mm == LuaValue::Nil)
		mm = GetMetafield(b, mmName, ls);
	if(mm == LuaValue::Nil)
		return std::make_tuple(LuaValue::Nil, false);
	
	ls->stack->Check(4);
	ls->stack->Push(mm);
	ls->stack->Push(a);
	ls->stack->Push(b);
	ls->Call(2, 1);
	return std::make_tuple(*ls->stack->Pop(), true);
}

LuaValue GetMetafield(const LuaValue& val, const String& fieldName, const LuaState* ls)
{
	LuaTablePtr mt = GetMetatable(val, ls);
	if(mt)
	{
		return *mt->Get(LuaValue(fieldName));
	}
	return LuaValue::Nil;
}