#pragma once
#include "public.h"
#include <tuple>

inline std::tuple<Int64, bool> ParseInteger(const String& str)
{
	Int64 number = 0;
	bool success = true;

	for(size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		if(c >= '0' && c <= '9')
		{
			number = number * 10 + c - '0';
		}
		else
		{
			success = false;
			break;
		}
	}

	return std::make_tuple(number, success);
}

inline std::tuple<Float64, bool> ParseFloat(const String& str)
{
	Float64 number = 0.0;
	Float64 t = 0.1;
	bool success = true;
	bool meetDot = false;

	for(size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		if(c >= '0' && c <= '9')
		{
			if(meetDot)
			{
				number = number + (c - '0') * t;
				t /= 10.0;
			}
			else
			{
				number = number * 10 + c - '0';
			}
		}
		else if(c == '.')
		{
			if(meetDot)
			{
				success = false;
				break;
			}
			meetDot = true;
		}
		else if(c == 'f' && i != str.size() - 1)
		{
			success = false;
			break;
		}
	}

	return std::make_tuple(number, success);
}

inline std::tuple<Int64, bool> FloatToInteger(Float64 f)
{
	Int64 i = (Int64)f;
	return std::make_tuple(i, (Float64)i == f);
}

inline std::tuple<Int64, bool> StringToInteger(const String& str)
{
	auto pair = ParseInteger(str);
	if(std::get<1>(pair))
		return pair;
	else
	{
		auto anotherPair = ParseFloat(str);
		if(std::get<1>(anotherPair))
			return FloatToInteger(std::get<0>(anotherPair));
	}
	return std::make_tuple(0, false);
}