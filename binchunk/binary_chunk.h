#pragma once
#include "type.h"
#include "vm/opcodes.h"
#include <vector>

struct BinaryChunk
{
	struct Head
	{
		Byte signature[4];
		Byte version;
		Byte format;
		Byte luacData[6];
		Byte cintSize;
		Byte sizetSize;
		Byte instructionSize;
		Byte luaIntegerSize;
		Byte luaNumberSize;
		Int64 luacInt;
		Float64 luacNum;
	};

	Head header;
	char sizeUpvalues;
	void* mainFunc;
};

constexpr static Byte LUA_SIGNATURE[4] = {0x1B, 0x4C, 0x75, 0x61};
constexpr static Byte LUAC_VERSION = 0x53;
constexpr static Byte LUAC_FORMAT = 0;
constexpr static Byte LUAC_DATA[6] = {0x19, 0x93, 0x0D,
	0x0A, 0x1A, 0x0A};
constexpr static Byte CINT_SIZE = 4;
constexpr static Byte CSIZE_SIZE = sizeof(size_t);
constexpr static Byte INSTRUCTION_SIZE = 4;
constexpr static Byte LUA_INTERGER_SIZE = 8;
constexpr static Byte LUA_NUMBER_SIZE = 8;
constexpr static Int64 LUAC_INT = 0x5678;
constexpr static Float64 LUAC_NUM = 370.5;

enum : Byte
{
	TAG_NIL = 0x00,
	TAG_BOOLEAN = 0x01,
	TAG_NUMBER = 0x03,
	TAG_INTEGER = 0x13,
	TAG_SHORT_STR = 0x04,
	TAG_LONG_STR = 0x14
};

struct Constant
{
	Byte tag;
	union
	{
		bool boolean;
		Byte byte;
		Int64 luaInteger;
		Float64 luaNum;
	};
	std::string str;
};

struct Upvalue
{
	char Instack;
	char Idx;
};

struct LocVar
{
	String VarName;
	UInt32 StartPC;
	UInt32 EndPC;
};

struct Prototype
{
	String Source;
	UInt32 LineDefined;
	UInt32 LastLineDefined;
	Byte NumParams;
	Byte IsVararg;
	Byte MaxStackSize;
	std::vector<UInt32> Code;
	std::vector<Constant> Constants;
	std::vector<Upvalue> Upvalues;
	std::vector<Prototype*> Protos;
	std::vector<UInt32> LineInfo;
	std::vector<LocVar> LocVars; 
	std::vector<std::string> UpvalueNames;

	void PrintHeader() const
	{
		String funcType = "main";
		if(LineDefined > 0)
			funcType = "function";
		String varargFlag = "";
		if(IsVararg)
			varargFlag += "+";
		printf("\n%s <%s:%d,%d> (%d instructions)\n", funcType.c_str(), Source.c_str(),
			LineDefined, LastLineDefined, Code.size());
		printf("%d%s parmas, %d slots, %d upvalues, ", NumParams, varargFlag.c_str(),
			MaxStackSize, UpvalueNames.size());
		printf("%d locals, %d constants, %d functions\n", LocVars.size(), Constants.size(), 
		Protos.size());
	}

	void PrintOperands(const Instruction& i) const
	{
		switch(i.OpMode())
		{
			case iABC:
			{
				auto abc = i.ABC();
				int a = std::get<0>(abc);
				int b = std::get<1>(abc);
				int c = std::get<2>(abc);
				printf("%d", a);
				if(i.BMode() != OpArgN)
				{
					if(b > 0xFF)
						printf(" %d", -1 - (b & 0xFF));
					else
						printf(" %d", b);
				}
				if(i.CMode() != OpArgN)
				{
					if(c > 0xFF)
						printf(" %d", -1 - (c & 0xFF));
					else
						printf(" %d", c);
				}
				break;
			}
			case iABx:
			{
				auto abx = i.ABx();
				int a = std::get<0>(abx);
				int bx = std::get<1>(abx);
				printf("%d", a);
				if(i.BMode() == OpArgK)
					printf(" %d", -1 - bx);
				else if(i.BMode() == OpArgU)
					printf(" %d", bx);
				break;
			}
			case iAsBx:
			{
				auto asbx = i.ABsBx();
				int a = std::get<0>(asbx);
				int sbx = std::get<1>(asbx);
				printf("%d %d", a, sbx);
				break;
			}
			case iAx:
			{
				int ax = i.Ax();
				printf("%d", -1 - ax);
				break;
			}
			default:
				break;
		}
	}

	void PrintCode() const
	{
		for(size_t pc = 0; pc < Code.size(); ++pc)
		{
			UInt32 c = Code[pc];
			String line = "-";
			if(LineInfo.size() > 0)
				line = Format::FormatString("%d", LineInfo[pc]);
			Instruction i{c};
			printf("\t%d\t[%s]\t%s \t", pc + 1, line.c_str(), i.OpName().c_str());
			PrintOperands(i);
			printf("\n");
		}
	}

	static String ConstantToString(const Constant& k)
	{
		switch (k.tag)
		{
			case TAG_NIL:
				return "";
			case TAG_BOOLEAN:
				return Format::FromBool(k.boolean);
			case TAG_NUMBER:
				return Format::FromFloat64(k.luaNum);
			case TAG_INTEGER:
				return Format::FromInt64(k.luaInteger);
			case TAG_SHORT_STR:
			case TAG_LONG_STR:
				return k.str;
			default:
				return "";
		}
	}

	String UpvalName(size_t idx) const
	{
		if(UpvalueNames.size() > idx)
			return UpvalueNames[idx];
		return "-";
	}

	void PrintDetail() const
	{
		printf("constants (%d):\n", Constants.size());
		for(size_t i = 0; i < Constants.size(); ++i)
		{
			const Constant& k = Constants[i];
			printf("\t%d\t%s\n", i + 1, ConstantToString(k).c_str());
		}
		printf("locals (%d):\n", LocVars.size());
		for(size_t i = 0; i < LocVars.size(); ++i)
		{
			const LocVar& locVar = LocVars[i];
			printf("\t%d\t%s\t%d\t%d\n", i, locVar.VarName.c_str(),
				locVar.StartPC + 1, locVar.EndPC + 1);
		}
		printf("upvalues (%d):\n", Upvalues.size());
		for(size_t i = 0; i < Upvalues.size(); ++i)
		{
			const Upvalue& upval = Upvalues[i];
			printf("\t%d\t%s\t\%d\t%d", i, UpvalName(i).c_str(), upval.Instack, upval.Idx);
		}
	}

	void List() const
	{
		PrintHeader();
		PrintCode();
		PrintDetail();
		for(Prototype* p : Protos)
		{
			p->List();
		}
	}
};