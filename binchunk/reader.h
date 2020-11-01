#pragma once
#include "binary_chunk.h"

using ByteArray = std::vector<Byte>;

inline void panic(const char* message)
{
	printf("%s\n", message);
	exit(0);
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

	void ReadBytes(size_t byteCount, Byte* dest, bool littleEndian = true)
	{
		if(pos + byteCount < data.size())
		{
			if(dest)
			{
				if(littleEndian)
				{
					dest[0] = data[pos++];
					dest[1] = data[pos++];
					dest[2] = data[pos++];
					dest[3] = data[pos++];
				}
				else
				{
					dest[3] = data[pos++];
					dest[2] = data[pos++];
					dest[1] = data[pos++];
					dest[0] = data[pos++];
				}
			}
			else
			{
				pos += 4;	
			}	
		}
	}

	UInt32 ReadUInt()
	{
		UInt32 num = 0;
		ReadBytes(sizeof(UInt32), (Byte*)&num, true);
		return num;
	}

	size_t ReadSizeT()
	{
		size_t num = 0;
		static_assert(sizeof(size_t) == 4, "size check");
		ReadBytes(sizeof(size_t), (Byte*)&num, true);
		return num;
	}

	UInt64 ReadLuaInteger()
	{
		UInt64 num = 0;
		ReadBytes(sizeof(UInt64), (Byte*)&num, true);
		return num;
	}

	double ReadLuaNumber()
	{
		UInt64 num = 0;
		static_assert(sizeof(UInt64) == sizeof(double), "size check");
		ReadBytes(sizeof(UInt64), (Byte*)&num, true);
		double dnum = 0.0;
		memcpy(&dnum, &num, sizeof(double));
		return dnum;
	}

	std::string ReadString()
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

		std::string bytes;
		bytes.resize(size);
		ReadBytes(size, const_cast<Byte*>(bytes.data()), true);
		return bytes;
	}

	void CheckHeader()
	{
		if(ReadUInt() != BinaryChunk::LUA_SIGNATURE)
		{
			panic("not a precompiled chunk!");
		}
		if(ReadByte() != BinaryChunk::LUAC_VERSION)
		{
			panic("version mismatch!");
		}
		if(ReadByte() != BinaryChunk::LUAC_FORMAT)
		{
			panic("format mismatch");
		}
	}
};

Prototype* Undump(const ByteArray& data)
{
	Reader reader {data};
	reader.CheckHeader();
	return nullptr;
}