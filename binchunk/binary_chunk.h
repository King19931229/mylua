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
};