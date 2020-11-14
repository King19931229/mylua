#pragma once
#include "binary_chunk.h"

using ByteArray = std::vector<Byte>;

inline bool bytesEqual(const Byte* lhs, const Byte* rhs, size_t len)
{
	if(lhs && rhs)
	{
		for(size_t i = 0; i < len; ++i)
			if(lhs[i] != rhs[i])
				return false;
		return true;
	}
	return false;
}

struct Reader
{
	ByteArray data;
	size_t pos;

	Reader(const ByteArray& _data)
	{
		data = _data;
		pos = 0;
	}

	Byte ReadByte()
	{
		if(pos < data.size())
		{
			return data[pos++];
		}
		return 0;
	}

	void _ReadBytes(size_t byteCount, Byte* dest, bool littleEndian = true)
	{
		if(pos + byteCount < data.size())
		{
			if(dest)
			{
				if(littleEndian)
				{
					for(size_t i = 0; i < byteCount; ++i)
						dest[i] = data[pos++];
				}
				else
				{
					for(size_t i = 0; i < byteCount; ++i)
						dest[byteCount - i - 1] = data[pos++];
				}
			}
			else
			{
				pos += byteCount;	
			}	
		}
	}

	ByteArray ReadBytes(size_t bytes, bool littleEndian = true)
	{
		ByteArray array;
		array.resize(bytes);
		_ReadBytes(bytes, array.data(), littleEndian);
		return array;
	}

	UInt32 ReadUInt()
	{
		UInt32 num = 0;
		_ReadBytes(sizeof(UInt32), (Byte*)&num, true);
		return num;
	}

	size_t ReadSizeT()
	{
		size_t num = 0;
		static_assert(sizeof(size_t) == 4, "size check");
		_ReadBytes(sizeof(size_t), (Byte*)&num, true);
		return num;
	}

	Int64 ReadLuaInteger()
	{
		Int64 num = 0;
		_ReadBytes(sizeof(Int64), (Byte*)&num, true);
		return num;
	}

	Float64 ReadLuaNumber()
	{
		Int64 num = 0;
		static_assert(sizeof(Int64) == sizeof(Float64), "size check");
		_ReadBytes(sizeof(Int64), (Byte*)&num, true);
		Float64 dnum = 0.0;
		memcpy(&dnum, &num, sizeof(Float64));
		return dnum;
	}

	String ReadString()
	{
		size_t size = ReadByte();
		if(size == 0)
		{
			return "";
		}
		if(size == 0xFF)
		{
			size = ReadSizeT();
		}
		--size;

		String bytes;
		bytes.resize(size);
		_ReadBytes(size, const_cast<Byte*>((const Byte*)bytes.data()), true);
		return bytes;
	}

	std::vector<UInt32> ReadCode()
	{
		UInt32 count = ReadUInt();
		std::vector<UInt32> code;
		code.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			code[i] = ReadUInt();
		}
		return code;
	}

	std::vector<Constant> ReadConstants()
	{
		UInt32 count = ReadUInt();
		std::vector<Constant> constants;
		constants.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			constants[i] = ReadConstant();
		}
		return constants;
	}

	Constant ReadConstant()
	{
		Byte tag = ReadByte();

		Constant constant;
		constant.tag = tag;
		switch(tag)
		{
			case TAG_NIL:
				break;
			case TAG_BOOLEAN:
				constant.boolean = ReadByte() != 0;
				break;
			case TAG_INTEGER:
				constant.luaInteger = ReadLuaInteger();
				break;
			case TAG_NUMBER:
				constant.luaNum = ReadLuaNumber();
				break;
			case TAG_SHORT_STR:
			case TAG_LONG_STR:
				constant.str = ReadString();
				break;
		}
		return constant;
	}

	std::vector<Upvalue> ReadUpvalues()
	{
		UInt32 count = ReadUInt();
		std::vector<Upvalue> upvalues;
		upvalues.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			upvalues[i].Instack = ReadByte();
			upvalues[i].Idx = ReadByte();
		}
		return upvalues;
	}

	std::vector<PrototypePtr> ReadProtos(const String& parentSource)
	{
		UInt32 count = ReadUInt();
		std::vector<PrototypePtr> protos;
		protos.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			protos[i] = ReadProtoType(parentSource);
		}
		return protos;
	}

	std::vector<UInt32> ReadLineInfo()
	{
		UInt32 count = ReadUInt();
		std::vector<UInt32> lineInfos;
		lineInfos.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			lineInfos[i] = ReadUInt();
		}
		return lineInfos;
	}

	std::vector<LocVar> ReadLocVars()
	{
		UInt32 count = ReadUInt();
		std::vector<LocVar> locVars;
		locVars.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			locVars[i] .VarName = ReadString();
			locVars[i].StartPC = ReadUInt();
			locVars[i].EndPC = ReadUInt();
		}
		return locVars;
	}

	std::vector<String> ReadUpvalueNames()
	{
		UInt32 count = ReadUInt();
		std::vector<String> names;
		names.resize(count);
		for(UInt32 i = 0; i < count; ++i)
		{
			names[i] = ReadString();
		}
		return names;
	}

	void CheckHeader()
	{
		if(!bytesEqual(ReadBytes(4).data(), LUA_SIGNATURE, 4))
		{
			panic("not a precompiled chunk!");
		}
		else if(ReadByte() != LUAC_VERSION)
		{
			panic("version mismatch!");
		}
		else if(ReadByte() != LUAC_FORMAT)
		{
			panic("format mismatch");
		}
		else if(!bytesEqual(ReadBytes(6).data(), LUAC_DATA, 6))
		{
			panic("corrupted!");
		}
		else if(ReadByte() != CINT_SIZE)
		{
			panic("int size mismatch");
		}
		else if(ReadByte() != CSIZE_SIZE)
		{
			panic("size_t size mismatch");
		}
		else if(ReadByte() != INSTRUCTION_SIZE)
		{
			panic("instruction size mismatch");
		}
		else if(ReadByte() != LUA_INTERGER_SIZE)
		{
			panic("lua_integer mismatch");
		}
		else if(ReadByte() != LUA_NUMBER_SIZE)
		{
			panic("lua_number mismatch");
		}
		else if(ReadLuaInteger() != LUAC_INT)
		{
			panic("endianness mismatch");
		}
		else if(ReadLuaNumber() != LUAC_NUM)
		{
			panic("float format mismatch");
		}
	}

	PrototypePtr ReadProtoType(const String& parentSource)
	{
		String source = ReadString();
		if(source == "")
		{
			source = parentSource;
		}
		PrototypePtr proto = PrototypePtr(new Prototype());
		proto->Source = source;
		proto->LineDefined = ReadUInt();
		proto->LastLineDefined = ReadUInt();
		proto->NumParams = ReadByte();
		proto->IsVararg = ReadByte();
		proto->MaxStackSize = ReadByte();
		proto->Code = ReadCode();
		proto->Constants = ReadConstants();
		proto->Upvalues = ReadUpvalues();
		proto->Protos = ReadProtos(source);
		proto->LineInfo = ReadLineInfo();
		proto->LocVars = ReadLocVars();
		proto->UpvalueNames = ReadUpvalueNames();
		return proto;		
	}
};

PrototypePtr Undump(const ByteArray& data)
{
	Reader reader {data};
	reader.CheckHeader();
	// skip Upvalue count
	reader.ReadByte();
	return reader.ReadProtoType("");
}