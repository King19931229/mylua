#pragma once
#include "type.h"
#include <tuple>

enum OpMode
{
	iABC,
	iABx,
	iAsBx,
	iAx
};

enum _OpCode
{
	/*----------------------------------------------------------------------
	name		args	description
	------------------------------------------------------------------------*/
	OP_MOVE,/*	A B	R(A) := R(B)					*/
	OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/
	OP_LOADKX,/*	A 	R(A) := Kst(extra arg)				*/
	OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/
	OP_LOADNIL,/*	A B	R(A), R(A+1), ..., R(A+B) := nil		*/
	OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/

	OP_GETTABUP,/*	A B C	R(A) := UpValue[B][RK(C)]			*/
	OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/

	OP_SETTABUP,/*	A B C	UpValue[A][RK(B)] := RK(C)			*/
	OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/
	OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/

	OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/

	OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

	OP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/
	OP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/
	OP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/
	OP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/
	OP_POW,/*	A B C	R(A) := RK(B) ^ RK(C)				*/
	OP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/
	OP_IDIV,/*	A B C	R(A) := RK(B) // RK(C)				*/
	OP_BAND,/*	A B C	R(A) := RK(B) & RK(C)				*/
	OP_BOR,/*	A B C	R(A) := RK(B) | RK(C)				*/
	OP_BXOR,/*	A B C	R(A) := RK(B) ~ RK(C)				*/
	OP_SHL,/*	A B C	R(A) := RK(B) << RK(C)				*/
	OP_SHR,/*	A B C	R(A) := RK(B) >> RK(C)				*/
	OP_UNM,/*	A B	R(A) := -R(B)					*/
	OP_BNOT,/*	A B	R(A) := ~R(B)					*/
	OP_NOT,/*	A B	R(A) := not R(B)				*/
	OP_LEN,/*	A B	R(A) := length of R(B)				*/

	OP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/

	OP_JMP,/*	A sBx	pc+=sBx; if (A) close all upvalues >= R(A - 1)	*/
	OP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
	OP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++		*/
	OP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++		*/

	OP_TEST,/*	A C	if not (R(A) <=> C) then pc++			*/
	OP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/

	OP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))) */
	OP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
	OP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

	OP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
				if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
	OP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

	OP_TFORCALL,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));	*/
	OP_TFORLOOP,/*	A sBx	if R(A+1) ~= nil then { R(A)=R(A+1); pc += sBx }*/

	OP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

	OP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx])			*/

	OP_VARARG,/*	A B	R(A), R(A+1), ..., R(A+B-2) = vararg		*/

	OP_EXTRAARG/*	Ax	extra (larger) argument for previous opcode	*/
};

enum OpArgMask
{
	OpArgN, /* argument is not used */
	OpArgU, /* argument is used */
	OpArgR, /* argument is a register or a jump offset */
	OpArgK /* argument is a constant or register/constant */
};

/* 9-bits 9-bits 8-bits 6-bits */
struct OpCode
{
	Byte testFlag; /* operator is a test (next instruction must be a jump) */
	Byte setAFlag; /* instruction set register A */
	Byte argBMode; /* B arg mode */
	Byte argCMode; /* C arg mode */
	Byte opMode; /* op mode */
	String name;
};

static OpCode opcodes[] =
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

static_assert(sizeof(opcodes) / sizeof(OpCode) == OP_EXTRAARG + 1, "size check");

struct Instruction
{
	UInt32 value;

	inline int Opcode() const
	{
		return int(value & 0x3F);
	}

	inline std::tuple<int, int, int>/* a, b, c */ ABC() const
	{
		return std::make_tuple(
			int(value >> 6 & 0xFF),
			int(value >> 23 & 0x1FF),
			int(value >> 14 & 0x1FF)
		);
	}

	std::tuple<int, int>/* a, bx */ ABx() const
	{
		return std::make_tuple(
			int(value >> 6 & 0xFF),
			int(value >> 14)
		);
	}

	static constexpr int MAXARG_BX = (1 << 18) - 1;
	static constexpr int MAXARG_sBX = MAXARG_BX >> 1;

	inline std::tuple<int, int> /* a, sbx */ ABsBx() const
	{
		auto temp = ABx();
		int a = std::get<0>(temp);
		int bx = std::get<1>(temp);
		return std::make_tuple(
			a,
			bx - MAXARG_sBX
		);
	}

	inline int Ax() const
	{
		return int(value >> 6);
	}

	inline String OpName() const
	{
		return opcodes[Opcode()].name;
	}

	inline Byte OpMode() const
	{
		return opcodes[Opcode()].opMode;
	}

	inline Byte BMode() const
	{
		return opcodes[Opcode()].argBMode;
	}

	inline Byte CMode() const
	{
		return opcodes[Opcode()].argCMode;
	}
};