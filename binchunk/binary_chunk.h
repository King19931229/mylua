#pragma once
#include <string>
#include <vector>

using Byte = char;
static_assert(sizeof(Byte) == 1, "size check");
using UInt32 = unsigned int;
static_assert(sizeof(UInt32) == 4, "size check");
using UInt64 = unsigned long long int;
static_assert(sizeof(UInt64) == 8, "size check");

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

	constexpr static UInt32 LUA_SIGNATURE = 0x61754C1B;
	constexpr static Byte LUAC_VERSION = 0x53;
	constexpr static Byte LUAC_FORMAT = 0;
	constexpr static Byte LUAC_DATA[6] = {(Byte)0x19, (Byte)0x93, (Byte)0x0D,
		(Byte)0x0A, (Byte)0x1A, (Byte)0x0A};
	constexpr static Byte CINT_SIZE = 4;
	constexpr static Byte CSIZE_SIZE = 8;
	constexpr static Byte INSTRUCTION_SIZE = 4;
	constexpr static Byte LUA_INTERGER_SIZE = 8;
	constexpr static Byte LUA_NUMBER_SIZE = 8;
	constexpr static UInt64 LUA_INT = 0x5678;
	constexpr static double LUA_NUM = 370.5;

	Head header;
	char sizeUpvalues;
	void* mainFunc;
};

// https://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
constexpr UInt32 BinaryChunk::LUA_SIGNATURE;
constexpr Byte BinaryChunk::LUAC_VERSION;
constexpr Byte BinaryChunk::LUAC_FORMAT;
constexpr Byte BinaryChunk::LUAC_DATA[6];
constexpr Byte BinaryChunk::CINT_SIZE;
constexpr Byte BinaryChunk::CSIZE_SIZE;
constexpr Byte BinaryChunk::INSTRUCTION_SIZE;
constexpr Byte BinaryChunk::LUA_INTERGER_SIZE;
constexpr Byte BinaryChunk::LUA_NUMBER_SIZE;
constexpr UInt64 BinaryChunk::LUA_INT;
constexpr double BinaryChunk::LUA_NUM;

enum : char
{
	TAG_NIL = 0x00,
	TAG_BOOLEAN = 0x01,
	TAG_NUMBER = 0x03,
	TAG_INTEGER = 0x13,
	TAG_SHORT_STR = 0x04,
	TAG_LONG_STR = 0x14
};

struct Upvalue
{
	char Instack;
	char Idx;
};

struct LocVar
{
	std::string VarName;
	UInt32 StartPC;
	UInt32 EndPC;
};

struct Prototype
{
	std::string Source;
	UInt32 LineDefined;
	UInt32 LastLineDefined;
	char NumParams;
	char IsVararg;
	char MaxStackSize;
	std::vector<UInt32> Code;
	std::vector<void*> Constants;
	std::vector<Upvalue> Upvalues;
	std::vector<Prototype*> Protos;
	std::vector<UInt32> LineInfo;
	std::vector<LocVar> LocVars; 
	std::vector<std::string> UpvalueNames;
};