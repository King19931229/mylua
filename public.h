#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <stdarg.h>

using Byte = unsigned char;
static_assert(sizeof(Byte) == 1, "size check");
using UInt32 = unsigned int;
static_assert(sizeof(UInt32) == 4, "size check");
using Int64 = long long int;
using UInt64 = unsigned long long int;
static_assert(sizeof(Int64) == 8, "size check");
using Float64 = double;
using String = std::string;
using ByteArray = std::vector<Byte>;

inline void panic(const char* message)
{
	printf("panic exit: %s\n", message);
	exit(0);
}

#define panic_cond(COND, message)\
do\
{\
	if(!(COND))\
		panic(message);\
}while(0);

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

		size_t requireBufferSize = VSNPRINTF(szBuffer, sizeof(szBuffer) - 1, pszFormat, list);
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
	static inline String FromFloat64(Float64 d) { return std::to_string(d); }
	static inline String FromInt64(Int64 i) { return std::to_string(i); }
	static inline String FromString(const String& s) { return s; };
};

// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
template <class T>
inline void HashCombine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*
** converts an integer to a "floating point byte", represented as
** (eeeeexxx), where the real value is (1xxx) * 2^(eeeee - 1) if
** eeeee != 0 and (xxx) otherwise.
*/
inline int Int2fb (int x)
{
	int e = 0;  /* exponent */
	if (x < 8) return x;
	while (x >= (8 << 4)) {  /* coarse steps */
	x = (x + 0xf) >> 4;  /* x = ceil(x / 16) */
	e += 4;
	}
	while (x >= (8 << 1)) {  /* fine steps */
	x = (x + 1) >> 1;  /* x = ceil(x / 2) */
	e++;
	}
	return ((e+1) << 3) | ((int)x - 8);
}

/* converts back */
inline int Fb2int (int x)
{
	return (x < 8) ? x : ((x & 7) + 8) << ((x >> 3) - 1);
}

template<typename Container>
Container Slice(Container& c, int beg, int end)
{
	beg = beg > 0 ? beg : c.size() + beg;
	end = end > 0 ? end : c.size() + end;
	// invalid bound
	if(beg < 0 || end < 0 || beg > end || end > (int)c.size())
	{
		panic("invalid bound");
		return Container();
	}
	Container res;
	res.resize(end - beg);
	for(int i = beg; i < end; ++i)
		res[i - beg] = c[i];
	return res;
}