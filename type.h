#pragma once
#include <string>

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