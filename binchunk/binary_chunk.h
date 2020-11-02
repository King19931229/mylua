#pragma once
#include <string>
#include <vector>

using Byte = unsigned char;
static_assert(sizeof(Byte) == 1, "size check");
using UInt32 = unsigned int;
static_assert(sizeof(UInt32) == 4, "size check");
using UInt64 = unsigned long long int;
static_assert(sizeof(UInt64) == 8, "size check");
using String = std::string;

#include <stdarg.h>

struct Format
{
	static String FormatString(const char* pszFormat, ...)
	{
	#if defined(_MSC_VER)
	#	define SNPRINTF sprintf_s
	#	define VSNPRINTF vsnprintf
	#	pragma warning(disable : 4996)
	#else
	#	define SNPRINTF snprintf
	#	define VSNPRINTF vsnprintf
	#endif
		String format;

		va_list list;
		va_start(list, pszFormat);

		char szBuffer[2048]; szBuffer[0] = '\0';

		int requireBufferSize = VSNPRINTF(szBuffer, sizeof(szBuffer) - 1, pszFormat, list);
		if (requireBufferSize > sizeof(szBuffer) - 1)
		{
			char *szAllocBuffer = new char[requireBufferSize + 1];
			memset(szAllocBuffer, 0, requireBufferSize + 1);
			VSNPRINTF(szAllocBuffer, requireBufferSize/* + 1 - 1 */, pszFormat, list);
			format = szAllocBuffer;
			delete[] szAllocBuffer;
		}
		else
		{
			format = szBuffer;
		}

		va_end(list);
		return format;

	#undef SNPRINTF
	#undef VSNPRINTF
	}

	static inline String FromBool(bool b) { return b ? "true" : "false"; }
	static inline String FromFloat64(double d) { return std::to_string(d); }
	static inline String FromUInt64(UInt64 i) { return std::to_string(i); }
	static inline String FromStirng(const String& s) { return s; };
};

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
		UInt64 luacInt;
		double luacNum;
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
constexpr static UInt64 LUAC_INT = 0x5678;
constexpr static double LUAC_NUM = 370.5;

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
		UInt64 luaInteger;
		double luaNum;
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
		printf("\n$s <%s:%d,%d> (%d instructions)\n", funcType.c_str(), Source,
			LineDefined, LastLineDefined, Code.size());
		printf("%d%s parmas, %d slots, %d upvalues, ", NumParams, varargFlag.c_str(),
			MaxStackSize, UpvalueNames.size());
		printf("%d locals, %d constants, %d functions\n", LocVars.size(), Constants.size(), 
		Protos.size());
	}

	void PrintCode() const
	{
		for(size_t pc = 0; pc < Code.size(); ++pc)
		{
			UInt32 c = Code[pc];
			String line = "-";
			if(LineInfo.size() > 0)
				line = Format::FormatString("%d", LineInfo[pc]);
			printf("\t%d\t[%s]\t0x%08X\n", pc + 1, line.c_str(), c);
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
				return Format::FromUInt64(k.luaInteger);
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