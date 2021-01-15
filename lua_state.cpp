#include "state/lua_value.h"
#include "state/lua_table.h"
#include "state/api_arith.h"
#include "vm/inst_misc.h"
#include "vm/inst_operators.h"
#include "vm/inst_table.h"
#include "vm/inst_call.h"
#include "vm/inst_upvalue.h"
#include "vm/opcodes.h"

String g_panic_message;

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
	,MAKE_OP_CODE(0, 0, OpArgN, OpArgU, iABC, TFORCALL, __call_insts__::tForCall)
	,MAKE_OP_CODE(0, 1, OpArgR, OpArgN, iAsBx, TFORLOOP, __call_insts__::tForLoop)
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
	mm = ::GetMetafield(a, mmName, ls);
	if(mm == LuaValue::Nil)
		mm = ::GetMetafield(b, mmName, ls);
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

LuaState::LuaState()
{
	stack = nullptr;
	registry = nullptr;

	coStatus = 0;
}

int LuaState::GetTop() const
{
	return (int)stack->top;
}

int LuaState::AbsIndex(int idx) const
{
	return stack->AbsIndex(idx);
}

bool LuaState::CheckStack(int n)
{
	stack->Check(n);
	return true;
}

void LuaState::Pop(int n)
{
	for(int i = 0; i < n; ++i)
		stack->Pop();
}

void LuaState::Copy(int fromIdx, int toIdx)
{
	LuaValue val = stack->Get(fromIdx);
	stack->Set(toIdx, val);
}

void LuaState::PushValue(int idx)
{
	LuaValue val = stack->Get(idx);
	stack->Push(val);
}

// Pop the value from the top, and set it back the stack by the index(so the index should be > 0)
void LuaState::Replace(int idx)
{
	LuaValue val = *stack->Pop();
	stack->Set(idx, val);
}

/*
Push the element of [idx, top] to the right by n units (if n >= 0)
otherwise push it to the left by n units
For array [1,2,3,4,5,6,7] move 4 units to the right
[1,2,3,4,5,6,7] -> [3,2,1,4,5,6,7] -> [3,2,1,7,6,5,4] -> [4,5,6,7,1,2,3]
It can be seen that p is the starting index,
t is the ending index, and shifted to the right by n units ==> r(p, t - n), r(t - n + 1, t), r(p, t)
On the contrary, if [1,2,3,4,5,6,7] move 4 units to the left
[1,2,3,4,5,6,7] -> [4,3,2,1,5,6,7] -> [4,3,2,1,7,6,5] -> [5,6,7,1,2,3,4]
It can be seen that p is the starting index,
t is the ending index, and shifted to the left by n units ==> r(p, p + n - 1), r(p + n, t), r(p, t)
*/
void LuaState::Rotate(int idx, int n)
{
	size_t t = stack->top - 1;
	size_t p = stack->AbsIndex(idx) - 1;
	size_t m = (n >= 0) ? (t - n) : (p - n - 1);
	stack->_Reverse(p, m);
	stack->_Reverse(m + 1, t);
	stack->_Reverse(p, t);
}

// Set capacity of the stack(idx means the capacity)
void LuaState::SetTop(int idx)
{
	size_t newTop = stack->AbsIndex(idx);
	if(newTop < 0)
	{
		panic("stack underflow");
	}
	int n = (int)stack->top - (int)newTop;
	if(n > 0)
	{
		for(int i = 0; i < n; ++i)
			stack->Pop();
	}
	else if(n < 0)
	{
		for(int i = 0; i > n; --i)
			stack->Push(LuaValue::Nil);
	}
}

// Pop the top of the stack and insert it into the specified position
void LuaState::Insert(int idx)
{
	Rotate(idx, 1);
}

void LuaState::Remove(int idx)
{
	Rotate(idx, -1);
	Pop(1);
}

void LuaState::PushNil() { stack->Push(LuaValue::Nil); }
void LuaState::PushBoolean(bool b) { stack->Push(LuaValue(b)); }
void LuaState::PushInteger(Int64 n) { stack->Push(LuaValue(n)); }
void LuaState::PushNumber(Float64 n) { stack->Push(LuaValue(n)); }
void LuaState::PushString(const String& str){ stack->Push(LuaValue(str)); }

LuaType LuaState::Type(int idx) const
{
	if(stack->IsValid(idx))
	{
		LuaValue value = stack->Get(idx);
		return value.tag;
	}
	return LUA_TNONE;
}

bool LuaState::IsNone(int idx) const { return Type(idx) == LUA_TNONE; }
bool LuaState::IsNil(int idx) const { return Type(idx) == LUA_TNIL; }
bool LuaState::IsNoneOrNil(int idx) const { return Type(idx) <= LUA_TNIL; }
bool LuaState::IsInteger(int idx) const
{
	LuaValue val = stack->Get(idx);
	if(val.tag == LUA_TNUMBER && !val.isfloat)
		return true;
	return false;
}
bool LuaState::IsNumber(int idx) const
{
	auto pair = ToNumberX(idx);
	return std::get<1>(pair);
}
bool LuaState::IsBoolean(int idx) const { return Type(idx) == LUA_TBOOLEAN; }
bool LuaState::IsTable(int idx) const { return Type(idx) == LUA_TTABLE; }
bool LuaState::IsThread(int idx) const { return Type(idx) == LUA_TTHREAD; }
bool LuaState::IsFunction(int idx) const { return Type(idx) == LUA_TFUNCTION; }

bool LuaState::IsString(int idx) const
{
	LuaType t = Type(idx);
	return t == LUA_TSTRING || t == LUA_TNUMBER;
}

void* LuaState::ToPointer(int idx) const
{
	if(idx < (int)stack->slots.size())
		return stack->slots[idx].get();
	return NULL;
}

bool LuaState::ToBoolean(int idx) const
{
	LuaValue value = stack->Get(idx);
	return ConvertToBoolean(value);
}

std::tuple<Float64, bool> LuaState::ToNumberX(int idx) const
{
	LuaValue value = stack->Get(idx);
	return ConvertToFloat(value);
}

Float64 LuaState::ToNumber(int idx) const
{
	auto pair = ToNumberX(idx);
	return std::get<0>(pair);
}

std::tuple<Int64, bool> LuaState::ToIntegerX(int idx) const
{
	LuaValue value = stack->Get(idx);
	return ConvertToInteger(value);
}

Int64 LuaState::ToInteger(int idx) const
{
	auto pair = ToIntegerX(idx);
	return std::get<0>(pair);
}

std::tuple<String, bool> LuaState::ToStringX(int idx)
{
	LuaValue val = stack->Get(idx);
	switch(val.tag)
	{
		case LUA_TSTRING: return std::make_tuple(val.str, true);
		case LUA_TNUMBER:
		{
			std::tuple<String, bool> ret;
			if(val.isfloat)
				ret = std::make_tuple(std::to_string(val.number), true);
			else
				ret = std::make_tuple(std::to_string(val.integer), true);
			stack->Set(idx, LuaValue(std::get<0>(ret)));
			return ret;
		}
		default: return std::make_tuple("", false);
	}
}

String LuaState::ToString(int idx)
{
	auto pair = ToStringX(idx);
	return std::get<0>(pair);
}

void LuaState::Arith(ArithOp op)
{
	LuaValue b = *stack->Pop();
	LuaValue a;
	if(op != LUA_OPUNM && op != LUA_OPBNOT)
		a = *stack->Pop();
	else
		a = b;
	Operator operaotr = operators[op];
	LuaValue res = _Arith(a, b, operaotr);
	if(res != LuaValue::Nil)
	{
		stack->Push(res);
		return;
	}
	// Call method only if value can not be converted into number
	const String& mm = operaotr.metamethod;
	auto metaRes = CallMetamethod(a, b, mm, this);
	if(std::get<1>(metaRes))
	{
		stack->Push(std::get<0>(metaRes));
		return;
	}
	panic("arithmetic error!");
}

bool LuaState::Compare(int idx1, int idx2, CompareOp op)
{
	LuaValue a = stack->Get(idx1);
	LuaValue b = stack->Get(idx2);
	switch (op)
	{
		case LUA_OPEQ: return _eq(a, b, this, false);
		case LUA_OPLT: return _lt(a, b, this, false);
		case LUA_OPLE: return _le(a, b, this, false);
		default: panic("invalid compare op!"); return false;
	}
}

bool LuaState::RawEqual(int idx1, int idx2)
{
	LuaValue a = stack->Get(idx1);
	LuaValue b = stack->Get(idx2);
	return _eq(a, b, this, true);
}

void LuaState::Len(int idx)
{
	LuaValue val = stack->Get(idx);
	if(val.IsString())
		stack->Push(LuaValue((Int64)val.str.length()));
	else
	{
		auto metaRes = CallMetamethod(val, val, "__len", this);
		if(std::get<1>(metaRes))
		{
			stack->Push(std::get<0>(metaRes));
		}
		else if(val.IsTable())
		{
			stack->Push(LuaValue((Int64)val.table->Len()));
		}
		else
		{
			panic("length error!");
		}
	}
}

int LuaState::RawLen(int idx)
{
	LuaValue val = stack->Get(idx);
	if(val.IsString())
		return (int)val.str.length();
	else if(val.IsTable())
		return (int)val.table->Len();
	else
		return 0;
}

void LuaState::Concat(int n)
{
	if(n == 0)
	{
		stack->Push(LuaValue(""));
	}
	else if(n > 1)
	{
		for(int i = 1; i < n; ++i)
		{
			if(IsString(-1) && IsString(-2))
			{
				String s1 = ToString(-2);
				String s2 = ToString(-1);
				stack->Pop();
				stack->Pop();
				stack->Push(LuaValue(s1 + s2));
				continue;
			}

			LuaValue b = *stack->Pop();
			LuaValue a = *stack->Pop();
			auto metaRes = CallMetamethod(a, b, "__concat", this);
			if(std::get<1>(metaRes))
			{
				stack->Push(std::get<0>(metaRes));
				continue;
			}

			panic("concatenation error!");
		}
	}
	// n == 1, do nothing
}

void LuaState::PushFString(const char* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	String msg = Format::FormatString(fmt, list);
	va_end(list);
	stack->Push(LuaValue(msg));
}

String LuaState::TypeName(LuaType tp)
{
	switch (tp)
	{
		case LUA_TNONE: return "no value";
		case LUA_TNIL: return "nil";
		case LUA_TBOOLEAN: return "boolean";
		case LUA_TNUMBER: return "number";
		case LUA_TSTRING: return "string";
		case LUA_TTABLE: return "table";
		case LUA_TFUNCTION: return "function";	
		case LUA_TTHREAD: return "thread";
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
		default: return "userdata";
	}
}


/*
interfaces for table
*/
void LuaState::CreateTable(int nArr, int nRec)
{
	LuaTablePtr t = NewLuaTable(nArr, nRec);
	stack->Push(LuaValue(t));
}

void LuaState::NewTable()
{
	CreateTable(0, 0);
}

LuaType LuaState::_GetTable(const LuaValue& t, const LuaValue& k, bool raw)
{
	if(t.IsTable())
	{
		LuaValue v = *t.table->Get(k);
		if(raw || v != LuaValue::Nil || !t.table->HasMetafield("__index"))
		{
			stack->Push(v);
			return v.tag;
		}
	}

	if(!raw)
	{
		LuaValue mf = ::GetMetafield(t, "__index", this);
		if(mf != LuaValue::Nil)
		{
			switch (mf.tag)
			{
				case LUA_TTABLE:
				{
					return _GetTable(mf, k, false);
				}
				case LUA_TFUNCTION:
				{
					stack->Push(mf);
					stack->Push(t);
					stack->Push(k);
					Call(2, 1);
					LuaValue v = stack->Get(-1);
					return v.tag;
				}
				default:
					break;
			}
		}
	}

	panic("index error!");
	return LUA_TNONE;
}

LuaType LuaState::GetTable(int idx)
{
	LuaValue t = stack->Get(idx);
	LuaValue k = *stack->Pop();
	return _GetTable(t, k, false);
}

LuaType LuaState::GetField(int idx, const String& k)
{
	LuaValue t = stack->Get(idx);
	return _GetTable(t, LuaValue(k), false);
}

LuaType LuaState::RawGetField(int idx, const String& k)
{
	LuaValue t = stack->Get(idx);
	return _GetTable(t, LuaValue(k), true);
}

LuaType LuaState::GetI(int idx, Int64 k)
{
	LuaValue t = stack->Get(idx);
	return _GetTable(t, LuaValue(k), false);
}

LuaType LuaState::RawGet(int idx)
{
	LuaValue t = stack->Get(idx);
	LuaValue k = *stack->Pop();
	return _GetTable(t, k, true);
}

LuaType LuaState::RawGetI(int idx, Int64 k)
{
	LuaValue t = stack->Get(idx);
	return _GetTable(t, LuaValue(k), true);
}

void LuaState::_SetTable(const LuaValue& t, const LuaValue& k, const LuaValue& v, bool raw)
{
	if(t.IsTable())
	{
		LuaTablePtr tbl = t.table;
		if(raw || *tbl->Get(k) != LuaValue::Nil || !tbl->HasMetafield("__newindex"))
		{
			t.table->Put(k, v);
			return;
		}
	}

	if(!raw)
	{
		LuaValue mf = ::GetMetafield(t, "__newindex", this);
		if(mf != LuaValue::Nil)
		{
			switch (mf.tag)
			{
				case LUA_TTABLE:
				{
					return _SetTable(mf, k, v, false);
				}
				case LUA_TFUNCTION:
				{
					stack->Push(mf);
					stack->Push(t);
					stack->Push(k);
					stack->Push(v);
					Call(3, 0);
					return;
				}
				default:
					break;
			}
		}
	}

	panic("index error!");
}

void LuaState::SetTable(int idx)
{
	LuaValue t = stack->Get(idx);
	LuaValue v = *stack->Pop();
	LuaValue k = *stack->Pop();
	_SetTable(t, k, v, false);
}

void LuaState::SetField(int idx, const String& k)
{
	LuaValue t = stack->Get(idx);
	LuaValue v = *stack->Pop();
	_SetTable(t, LuaValue(k), LuaValue(v), false);
}

void LuaState::SetI(int idx, Int64 i)
{
	LuaValue t = stack->Get(idx);
	LuaValue v = *stack->Pop();
	_SetTable(t, LuaValue(i), LuaValue(v), false);
}

void LuaState::RawSet(int idx)
{
	LuaValue t = stack->Get(idx);
	LuaValue v = *stack->Pop();
	LuaValue k = *stack->Pop();
	_SetTable(t, LuaValue(k), LuaValue(v), true);
}

void LuaState::RawSetI(int idx, Int64 i)
{
	LuaValue t = stack->Get(idx);
	LuaValue v = *stack->Pop();
	_SetTable(t, LuaValue(i), LuaValue(v), true);
}

void LuaState::PushLuaStack(LuaStackPtr s)
{
	s->prev = stack;
	stack = s;
}

void LuaState::PopLuaStack()
{
	LuaStackPtr s = stack;
	stack = stack->prev;
	s->prev = nullptr;
}

bool LuaState::IsBinaryChunk(const ByteArray& chunk)
{
	if(chunk.size() >= 4)
	{
		Byte signature[4];
		static_assert(sizeof(signature) == sizeof(UInt32), "size check");
		memcpy(signature, chunk.data(), sizeof(signature));
		return memcmp(signature, LUA_SIGNATURE, sizeof(signature)) == 0;
	}
	return false;
}

int LuaState::Load(const ByteArray& chunk, const String& chunkName, const String& mode)
{
	PrototypePtr proto = nullptr;
	if(IsBinaryChunk(chunk))
	{
		proto = Undump(chunk);
	}
	else
	{
		String strChunk;
		strChunk.reserve(chunk.size());
		for(Byte ch : chunk)
		{
			strChunk += ch;
		}
		proto = Compile(strChunk, chunkName);
	}
	ClosurePtr closure = NewLuaClosure(proto);
	stack->Push(LuaValue(closure));
	if(proto->Upvalues.size() > 0)
	{
		LuaValuePtr env = registry->Get(LuaValue(LUA_RIDX_GLOBALS));
		closure->upvals[0] = UpValue(env);
	}
	return LUA_OK;
}

void LuaState::RunLuaClosure()
{
	DEBUG_PRINT("Run Lua Closure");
	while(true)
	{
		Instruction inst = Instruction(Fetch());
#if DEBUG_PRINT_ENABLE
		LuaStackPtr counter = stack;
		int depth = 0;
		while (counter->prev)
		{
			++depth;
			counter = counter->prev;
		}
		for (int i = 0; i < depth * 5; ++i)
			printf("(");
		printf("thread:0x%x stack:0x%x pc:%d\n", (size_t)this, (size_t)stack.get(), stack->pc - 1);
		puts("----------Stack Before Execution----------");
		PrintStack(*this);
		puts("----------Stack Before Execution----------");
		DEBUG_PRINT("%s", inst.OpName().c_str());
		Prototype::PrintOperands(inst);
		puts("");
#endif
		inst.Execute(this);
#if DEBUG_PRINT_ENABLE
		puts("----------Stack After Execution----------");
		PrintStack(*this);
		puts("----------Stack After Execution----------");
		printf("thread:0x%x stack:0x%x pc:%d", (size_t)this, (size_t)stack.get(), stack->pc - 1);
		for (int i = 0; i < depth * 5; ++i)
			printf(")");
		puts("");
#endif
		if(inst.Opcode() == OP_RETURN)
			break;
	}
}

void LuaState::CallLuaClosure(int nArgs, int nResults, ClosurePtr c)
{
	int nRegs = (int)c->proto->MaxStackSize;
	int nParams = (int)c->proto->NumParams;
	bool isVararg = c->proto->IsVararg == 1;

	LuaStackPtr newStack = NewLuaStack(nRegs + LUA_MINSTACK, stack->state);
	// Assign the function
	newStack->closure = c;

	// a = func(1,2,3,...)
	// a(1,2,3,4,5) nArgs = 5, nParams = 3
	// nArgs(which is all arguments) contains nParams(which is non varargs arguments)
	LuaValueArray funcAndArgs = stack->PopN(nArgs + 1);
	// Push the non varargs arguments
	newStack->PushN(Slice(funcAndArgs, 1, funcAndArgs.size()), nParams);
	panic_cond(nRegs >= nParams, "nRegs is less than nParams");
	newStack->top = nRegs;
	if (nArgs > nParams && isVararg)
	{
		// Assign the varargs arguments
		newStack->varargs = Slice(funcAndArgs, nParams + 1, funcAndArgs.size());
	}

	PushLuaStack(newStack);
	RunLuaClosure();
	PopLuaStack();

	panic_cond(!stack->closure || stack->closure->cFunc || stack->top == stack->closure->proto->MaxStackSize,
		"return values must locate on MaxStackSize of current call frame");

	if (nResults != 0)
	{
		// Pop the return values from the newStack
		LuaValueArray results = newStack->PopN(newStack->top - nRegs);
		stack->Check((int)results.size());
		// nResults not "(int)results.size()" for some ticky usage
		stack->PushN(results, nResults);
	}
}

void LuaState::CallCClosure(int nArgs, int nResults, ClosurePtr c)
{
	LuaStackPtr newStack = NewLuaStack(nArgs + LUA_MINSTACK, stack->state);
	newStack->closure = c;

	LuaValueArray args = stack->PopN(nArgs);
	newStack->PushN(args, nArgs);
	// Throw away the C closure
	stack->Pop();

	PushLuaStack(newStack);
	int r = c->cFunc(this);
	PopLuaStack();

	if(nResults != 0)
	{
		LuaValueArray results = newStack->PopN(r);
		newStack->Check((int)results.size());
		stack->PushN(results, nResults);
	}
}

void LuaState::Call(int nArgs, int nResults)
{
	// closure func
	LuaValue val = stack->Get(-(nArgs + 1));

	if(!val.IsClosure())
	{
		LuaValue mf = ::GetMetafield(val, "__call", this);
		if(mf != LuaValue::Nil)
		{
			if(mf.IsClosure())
			{
				stack->Push(mf);
				Insert(-(nArgs + 2));
				nArgs += 1;
				val = mf;
			}
		}
	}

	if(val.IsClosure())
	{
		ClosurePtr c = val.closure;
		if(c->proto)
		{
			DEBUG_PRINT("call %s<%d,%d>", c->proto->Source.c_str(),
				c->proto->LineDefined,
				c->proto->LastLineDefined);
		}
		else
		{
			auto it = cFuncNames.find(c->cFunc);
			if (it != cFuncNames.end() && it->second.length() > 0)
			{
				DEBUG_PRINT("%s", it->second.c_str());
			}
			else
			{
				DEBUG_PRINT("call c function");
			}
		}

		if(c->proto != nullptr)
			CallLuaClosure(nArgs, nResults, c);
		else
			CallCClosure(nArgs, nResults, c);
	}
	else
	{
		panic("not a function");
	}
}

void LuaState::PushCClosure(CFunction c, int n)
{
	ClosurePtr closure = NewCClosure(c, n);
	for(int i = n; i > 0; --i)
	{
		LuaValuePtr val = stack->Pop();
		closure->upvals[i - 1] = UpValue(val);
	}
	stack->Push(LuaValue(closure));
}

void LuaState::PushCFunction(CFunction c)
{
	PushCClosure(c, 0);
}

bool LuaState::IsCFunction(int idx)
{
	LuaValue val = stack->Get(idx);
	if(val.IsClosure())
	{
		return val.closure->cFunc != nullptr;
	}
	return false;
}

CFunction LuaState::ToCFunction(int idx)
{
	LuaValue val = stack->Get(idx);
	if(val.IsClosure())
	{
		return val.closure->cFunc;
	}
	return nullptr;
}

bool LuaState::IsProto(int idx)
{
	LuaValue val = stack->Get(idx);
	if (val.IsClosure())
	{
		return val.closure->proto != nullptr;
	}
	return false;
}

PrototypePtr LuaState::ToProto(int idx)
{
	LuaValue val = stack->Get(idx);
	if (val.IsClosure())
	{
		return val.closure->proto;
	}
	return nullptr;
}

void LuaState::PushGlobalTable()
{
	LuaValue global = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
	stack->Push(global);
}

LuaType LuaState::GetGlobal(const String& name)
{
	LuaValue t = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
	return _GetTable(t, LuaValue(name), true);
}

void LuaState::SetGlobal(const String& name)
{
	LuaValue t = *registry->Get(LuaValue(LUA_RIDX_GLOBALS));
	LuaValue v = *stack->Pop();
	_SetTable(t, LuaValue(name), v, true);
}

void LuaState::Register(const String& name, CFunction f)
{
	PushCFunction(f);
	SetGlobal(name);
}

bool LuaState::GetMetatable(int idx)
{
	LuaValue val = stack->Get(idx);
	LuaTablePtr mt = ::GetMetatable(val, this);
	if(mt)
	{
		stack->Push(LuaValue(mt));
		return true;
	}
	else
	{
		return false;
	}
}

void LuaState::SetMetatable(int idx)
{
	LuaValue val = stack->Get(idx);
	LuaValue mtVal = *stack->Pop();
	if(mtVal == LuaValue::Nil)
	{
		::SetMetatable(val, LuaTable::NilPtr, this);
	}
	else if(mtVal.IsTable())
	{
		::SetMetatable(val, mtVal.table, this);
	}
	else
	{
		panic("table expected!"); // todo
	}
}

bool LuaState::Next(int idx)
{
	LuaValue val = stack->Get(idx);
	if(val.IsTable())
	{
		LuaValue key = *stack->Pop();
		LuaTablePtr tbl = val.table;
		LuaValue nextKey = tbl->NextKey(key);
		if(nextKey != LuaValue::Nil)
		{
			stack->Push(nextKey);
			stack->Push(*tbl->Get(nextKey));
			return true;
		}
		return false;
	}
	panic("table expected!");
	return false;
}

int LuaState::Error()
{
	LuaValue err = *stack->Pop();
	panic(err.str.c_str());
	return LUA_ERRRUN;
}

void LuaState::_Call(int nArgs, int nResults)
{
	Call(nArgs, nResults);
}

int LuaState::PCall(int nArgs, int nResults, int msgh)
{
	int status = LUA_ERRRUN;
	status = _ProtectedRun(&LuaState::_Call, nArgs, nResults);
	return status;
}

/*
interfaces for luavm
*/
int LuaState::PC() const { return stack->pc; }

void LuaState::AddPC(int n) { stack->pc += n; }

UInt32 LuaState::Fetch()
{
	panic_cond(stack, "stack must not empty");
	panic_cond(stack->closure, "closure must not empty");
	panic_cond(stack->closure->proto, "proto must not empty");
	panic_cond(stack->pc < (int)stack->closure->proto->Code.size(), "pc out of bound");
	return stack->closure->proto->Code[stack->pc++];
}

void LuaState::GetConst(int idx)
{
	Constant c = stack->closure->proto->Constants[idx];
	switch (c.tag)
	{
		case TAG_NIL: stack->Push(LuaValue::Nil); break;
		case TAG_BOOLEAN: stack->Push(LuaValue(c.boolean)); break;
		case TAG_NUMBER: stack->Push(LuaValue(c.luaNum)); break;
		case TAG_INTEGER: stack->Push(LuaValue(c.luaInteger)); break;
		case TAG_SHORT_STR:
		case TAG_LONG_STR: stack->Push(LuaValue(c.str)); break;
	}
}

void LuaState::GetRK(int rk)
{
	if(rk > 0xFF)
		GetConst(rk & 0xFF);
	else
		PushValue(rk + 1);
}

int LuaState::RegisterCount() const { return stack->closure->proto->MaxStackSize;	}

void LuaState::LoadVararg(int n)
{
	if(n < 0)
		n = stack->varargs.size();
	stack->Check(n);
	stack->PushN(stack->varargs, n);
}

void LuaState::LoadProto(int idx)
{
	PrototypePtr subProto = stack->closure->proto->Protos[idx];
	ClosurePtr closure = NewLuaClosure(subProto);
	stack->Push(LuaValue(closure));

	for(size_t i = 0; i < subProto->Upvalues.size(); ++i)
	{
		const Upvalue& uvInfo = subProto->Upvalues[i];
		int uvIdx = uvInfo.Idx;

		if(uvInfo.Instack == 1)
		{
			auto it = stack->openuvs.find(uvIdx);
			if(it == stack->openuvs.end())
			{
				closure->upvals[i] = UpValue(stack->slots[uvIdx]);
				stack->openuvs[uvIdx] = closure->upvals[i];
			}
			else
			{
				closure->upvals[i] = it->second;
			}
		}
		else
		{
			closure->upvals[i] = stack->closure->upvals[uvIdx];
		}
	}
}

void LuaState::CloseUpvalues(int a)
{
	for(auto it = stack->openuvs.begin(); it != stack->openuvs.end();)
	{
		int i = it->first;
		if(i >= a - 1)
		{
			it = stack->openuvs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

/* Coroutine */

LuaStatePtr LuaState::NewThread()
{
	LuaStatePtr t = LuaStatePtr(new LuaState());
	t->registry = registry;
	t->PushLuaStack(NewLuaStack(LUA_MINSTACK, t.get()));
	stack->Push(LuaValue(t));
	return t;
}

bool LuaState::IsMainThread()
{
	LuaValue mainThread = *registry->Get(LuaValue(LUA_RIDX_MAINTHREAD));
	return mainThread.state.get() == this;
}

void LuaState::_FixYieldStack(int nArgs)
{
	// Resume values have been pushed into the current stack
	LuaValueArray arguments = stack->PopN(nArgs);
	// Pop the yield call stack
	PopLuaStack();
	// Check the stack size
	stack->Check(nArgs);
	// Push the resume arguments into the stack as yield returns
	stack->PushN(arguments, nArgs);
	// Recover the pc
	--stack->pc;

	Instruction inst = Instruction(Fetch());
	int opCode = inst.Opcode();
	panic_cond(opCode == OP_CALL || opCode == OP_TAILCALL, "must fix a function call");

	// Finish the unfinished process
	if (opCode == OP_CALL)
	{
		auto abc = inst.ABC();
		int a = std::get<0>(abc) + 1;
		// int b = std::get<1>(abc);
		int c = std::get<2>(abc);
		__call_insts__::_popResults(a, c, this);
	}
	else if (opCode == OP_TAILCALL)
	{
		auto ab_ = inst.ABC();
		int a = std::get<0>(ab_) + 1;
		// int b = std::get<1>(ab_);
		int c = 0;
		__call_insts__::_popResults(a, c, this);
	}
}

int LuaState::_ProtectedRun(FunctionCall func, int nArgs, int nResults)
{
	LuaStackPtr caller = stack;
	try
	{
		(this->*func)(nArgs, nResults);
	}
	catch (int st)
	{
		if (st == LUA_ERRRUN || st == LUA_ERRERR)
		{
			while (stack != caller)
			{
				PopLuaStack();
			}
			stack->Push(LuaValue(g_panic_message));
		}
		return st;
	}
	catch (...)
	{
		printf("panic\n");
		exit(0);
	}
	return LUA_OK;
}

void LuaState::_Resume(int nArgs, int nResults)
{
	// start coroutine	
	if (coStatus == LUA_OK)
	{
		Call(nArgs, nResults);
	}
	// resume coroutine
	else
	{
		coStatus = LUA_OK;
		_FixYieldStack(nArgs);

		LuaStackPtr newStack = stack;

		if (stack->closure->proto)
		{
			int nRegs = (int)stack->closure->proto->MaxStackSize;

			RunLuaClosure();
			PopLuaStack();

			panic_cond(!stack->closure || stack->closure->cFunc || stack->top == stack->closure->proto->MaxStackSize,
				"return values must locate on MaxStackSize of current call frame");

			if (nResults != 0)
			{
				// Pop the return values from the newStack
				LuaValueArray results = newStack->PopN(newStack->top - nRegs);
				stack->Check((int)results.size());
				// nResults not "(int)results.size()" for some ticky usage
				stack->PushN(results, nResults);
			}
		}
		else
		{
			int r = stack->closure->cFunc(this);
			PopLuaStack();

			if (nResults != 0)
			{
				LuaValueArray results = stack->PopN(r);
				newStack->Check((int)results.size());
				stack->PushN(results, nResults);
			}
		}
	}
}

int LuaState::Resume(LuaState* fromState, int nArgs)
{
	LuaStackPtr caller = stack;
	// TODO
	coStatus = _ProtectedRun(&LuaState::_Resume, nArgs, -1);
	return coStatus;
}

void LuaState::Yield(int nResults)
{
	coStatus = LUA_YIELD;
	panic_cond(nResults == GetTop(), "must yield all arguments");
	throw LUA_YIELD;
}

int LuaState::Status()
{
	return coStatus;
}

bool LuaState::IsYieldable()
{
	if(IsMainThread())
		return false;
	
	return coStatus != LUA_YIELD;
}

LuaStatePtr LuaState::ToThread(int idx)
{
	LuaValue val = stack->Get(idx);
	// val and val.state could be null or not
	return val.state;
}

void LuaState::XMove(LuaState* to, int n)
{
	LuaValueArray vals = stack->PopN(n);
	to->stack->PushN(vals, n);
}

bool LuaState::GetStack()
{
	return stack->prev != nullptr;
}

bool LuaState::PushThread()
{
	// stack->Push(this);
	panic("todo");
	return IsMainThread();
}