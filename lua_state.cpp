#include "state/lua_value.h"
#include "state/api_arith.h"
#include "vm/opcodes.h"

const LuaValue LuaValue::NoValue(LUA_TNONE);
const LuaValue LuaValue::Nil(LUA_TNIL);

const OpCode opcodes[47] =
{
#define MAKE_OP_CODE(T, A, B, C, mode, name) OpCode{T, A, B, C, mode, #name}
	MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, MOVE)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgN, iABx, LOADK)
	,MAKE_OP_CODE(0, 1, OpArgN, OpArgN, iABx, LOADKX)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, LOADBOOL)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, LOADNIL)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, GETUPVAL)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgK, iABC, GETTABUP)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgK, iABC, GETTABLE)
	,MAKE_OP_CODE(0, 0, OpArgK, OpArgK, iABC, SETTABUP)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgN, iABC, SETUPVAL)
	,MAKE_OP_CODE(0, 0, OpArgK, OpArgK, iABC, SETTABLE)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, NEWTABLE)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgK, iABC, SELF)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, ADD)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SUB)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, MUL)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, MOD)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, POW)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, DIV)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, IDIV)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BAND)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BOR)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, BXOR)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SHL)
	,MAKE_OP_CODE(0, 1, OpArgK, OpArgK, iABC, SHR)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, UNM)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, BNOT)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, NOT)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iABC, LEN)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgR, iABC, CONCAT)
	,MAKE_OP_CODE(0, 0, OpArgR, OpArgN, iAsBx, JMP)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, EQ)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, LT)
	,MAKE_OP_CODE(1, 0, OpArgK, OpArgK, iABC, LE)
	,MAKE_OP_CODE(1, 0, OpArgN, OpArgU, iABC, TEST)
	,MAKE_OP_CODE(1, 1, OpArgR, OpArgU, iABC, TESTSET)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, CALL)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgU, iABC, TAILCALL)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgN, iABC, RETURN)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, FORLOOP)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, FORPREP)
	,MAKE_OP_CODE(0, 0, OpArgN, OpArgU, iABC, TFORCALL)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, TFORLOOP)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgU, iABC, SETLIST)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABx, CLOSURE)
	,MAKE_OP_CODE(0, 1, OpArgU, OpArgN, iABC, VARARG)
	,MAKE_OP_CODE(0, 0, OpArgU, OpArgU, iAx, EXTRAARG)
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
	static Float64 pow(Float64 a, Float64 b) { return pow(a, b); }
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
	Operator{__funcs__::iadd, __funcs__::fadd},
	Operator{__funcs__::isub, __funcs__::fsub},
	Operator{__funcs__::imul, __funcs__::fmul},
	Operator{__funcs__::imod, __funcs__::fmod},
	Operator{nullptr, __funcs__::pow},
	Operator{nullptr, __funcs__::div},
	Operator{__funcs__::iidiv, __funcs__::fidiv},
	Operator{__funcs__::band, nullptr},
	Operator{__funcs__::bor, nullptr},
	Operator{__funcs__::bxor, nullptr},
	Operator{__funcs__::shl, nullptr},
	Operator{__funcs__::shr, nullptr},
	Operator{__funcs__::iunm, __funcs__::funm},
	Operator{__funcs__::bnot, nullptr},
};