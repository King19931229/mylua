#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
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
using StringArray = std::vector<String>;

#ifdef _WIN32
#	ifdef _WIN64
#		define LUA_X64
#	else
#		define LUA_X86
#endif
#else
#	ifdef __x86_64__
#		define LUA_X64
#	else
#		define LUA_X86
#endif
#endif

struct LuaTable;
using LuaTablePtr = std::shared_ptr<LuaTable>;

struct Closure;
using ClosurePtr = std::shared_ptr<Closure>;

struct LuaValue;
using LuaValuePtr = std::shared_ptr<LuaValue>;

struct LuaStack;
using LuaStackPtr = std::shared_ptr<LuaStack>;

struct LuaState;
using LuaStatePtr = std::shared_ptr<LuaState>;

struct Prototype;
using PrototypePtr = std::shared_ptr<Prototype>;

struct UpValue;
using UpValuePtr = std::shared_ptr<UpValue>;

using LuaVM = LuaState;

struct RTTI
{
	virtual ~RTTI() {}
	template<typename T>
	bool IsA()
	{
		return dynamic_cast<T*>(this) != nullptr;
	}
	template<typename T>
	T* Cast()
	{
		return dynamic_cast<T*>(this);
	}
};

extern String g_panic_message;

inline void panic(const char* message)
{
	g_panic_message = message;
	// printf("panic exit: %s\n", message);
	// exit(0);
	printf("%s\n", message);
	// LUA_ERRRUN
	throw 2;
}

inline void warning(const char* message)
{
	printf("warning: %s\n", message);
}

// #define VERBOSE_PRINT

#if defined(_DEBUG) && defined(VERBOSE_PRINT)
#define DEBUG_PRINT_ENABLE 1
#else
#define DEBUG_PRINT_ENABLE 0
#endif

#if DEBUG_PRINT_ENABLE
	#define DEBUG_PRINT(...) printf("[Debug]: "); printf(__VA_ARGS__); printf("\n");
#else
	#define DEBUG_PRINT(...)
#endif

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
			VSNPRINTF(szAllocBuffer, requireBufferSize, pszFormat, list);
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

struct StringUtil
{
	static String Replace(const String& input, const String& replacee, const String& replacer)
	{
		String result = input;
		String::size_type pos = result.find(replacee, 0);
		while(pos != String::npos)
		{
			result.replace(pos, replacee.length(), replacer);
			pos = result.find(replacee, 0);
		}
		return result;
	}

	static StringArray Split(const String& input, const String& splitChars)
	{
		StringArray splitResult;

		if (!input.empty())
		{
			size_t lastPos = 0;
			size_t curPos = 0;
			while (curPos < input.length())
			{
				lastPos = curPos;
				curPos = input.find_first_of(splitChars, lastPos);
				if (curPos != std::string::npos)
				{
					splitResult.push_back(input.substr(lastPos, curPos - lastPos));
					++curPos;
					if (curPos == input.length())
					{
						splitResult.push_back("");
						break;
					}
				}
				else
				{
					splitResult.push_back(input.substr(lastPos));
					break;
				}
			}
		}

		return splitResult;
	}

	static String Strip(const String& src, const String& stripChars, bool left, bool right)
	{
		String result = src;
		if(!src.empty())
		{
			String::size_type startPos = 0;
			String::size_type endPos = src.length() - 1;

			if(left)
			{
				startPos = src.find_first_not_of(stripChars);
			}
			if(right)
			{
				endPos = src.find_last_not_of(stripChars);
			}
			if(startPos != std::string::npos)
			{
				if(endPos != std::string::npos)
				{
					result = src.substr(startPos, endPos - startPos + 1);
				}
				else
				{
					result = src.substr(startPos);
				}
			}
			else
			{
				result = "";
			}
		}
		return result;
	}

	static bool StartsWith(const String& src, const String& chars)
	{
		if(src.length() >= chars.length() && src.substr(0, chars.length()) == chars)
			return true;
		return false;
	}

	static bool EndsWith(const String& src, const String& chars)
	{
		if(src.length() >= chars.length() && src.substr(src.length() - chars.length()) == chars)
			return true;
		return false;
	}

	static bool Upper(const String& src, String& dest)
	{
		dest = src;
		std::transform(dest.begin(), dest.end(), dest.begin(), toupper);
		return true;
	}

	bool Lower(const String& src, String& dest)
	{
		dest = src;
		std::transform(dest.begin(), dest.end(), dest.begin(), tolower);
		return true;
	}
};

// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
// https://github.com/HowardHinnant/hash_append/issues/7
template <typename T>
inline void HashCombine(std::uint16_t& seed, const T& val)
{
	seed ^= std::hash<T>{}(val) + 0x9e37U + (seed<<3) + (seed>>1);
}

template <typename T>
inline void HashCombine(std::uint32_t& seed, const T& val)
{
	seed ^= std::hash<T>{}(val) + 0x9e3779b9U + (seed<<6) + (seed>>2);
}

template <class T>
inline void HashCombine(std::uint64_t& seed, const T& val)
{
	seed ^= std::hash<T>{}(val) + 0x9e3779b97f4a7c15LLU + (seed<<12) + (seed>>4);
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
	beg = beg > 0 ? beg : (int)c.size() + beg;
	end = end > 0 ? end : (int)c.size() + end;
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