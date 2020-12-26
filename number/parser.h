#pragma once
#include "public.h"
#include <tuple>
#include <cmath>

inline std::tuple<Int64, bool> ParseInteger(const String& str)
{
	Int64 number = 0;
	Int64 sign = 1;

	bool success = true;

	if(str.length() && str[0] == '-')
		sign = -1;

	if(str.find("0x") == 0 || str.find("0X") == 0)
	{
		for(size_t i = 2; i < str.size(); ++i)
		{
			char c = str[i];
			if(c >= '0' && c <= '9')
			{
				number = number * 16 + c - '0';
			}
			else if(c >= 'a' && c <= 'f')
			{
				number = number * 16 + c - 'a' + 10;
			}
			else if(c >= 'A' && c <= 'F')
			{
				number = number * 16 + c - 'A' + 10;
			}
			else
			{
				success = false;
				break;
			}
		}
	}
	else
	{
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
	}

	return std::make_tuple(number * sign, success);
}

inline std::tuple<Float64, bool> ParseFloat(const String& str)
{
	Float64 number = 0;
	Float64 sign = 1;
	bool success = true;
	bool meetDot = false;
	bool meetE = false;

	if(str.length() && str[0] == '-')
		sign = -1;

	if(str.find("0x") == 0 || str.find("0X") == 0)
	{
		for(size_t i = 2; i < str.size(); ++i)
		{
			// todo t is 1/10 or 1/16
			Float64 t = 0.1;
			char c = str[i];
			if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
			{
				int n = 0;
				if(c >= '0' && c <= '9')
					n = c - '0';
				else if(c >= 'a' && c <= 'f')
					n = c - 'a' + 10;
				else if(c >= 'A' && c <= 'F')
					n = c - 'A' + 10;

				if(meetDot)
				{
					if(n >= 10)
					{
						success = false;
						break;
					}
					number = number + n * t;
					t /= 10.0;
				}
				else if(meetE)
				{
					auto pair = ParseInteger(str.substr(i));
					if(!std::get<1>(pair))
					{
						success = false;
						break;
					}
					Float64 e = std::get<0>(pair);
					number = number * pow(2.0, e);
					// fast break
					i = str.size() - 1;
				}
				else
				{
					number = number * 16 + n;
				}
			}
			else if(c == '.')
			{
				if(meetDot || meetE)
				{
					success = false;
					break;
				}
				meetDot = true;
			}
			else if(c == 'p' || c == 'P')
			{
				if(meetE)
				{
					success = false;
					break;
				}
				meetE = true;
				meetDot = false;
			}
			else if(c == 'f' && i != str.size() - 1)
			{
				success = false;
				break;
			}
		}
	}
	else
	{
		Float64 t = 0.1;
		for(size_t i = 0; i < str.size(); ++i)
		{
			char c = str[i];
			if(c >= '0' && c <= '9')
			{
				int n = (c - '0');
				if(meetDot)
				{
					number = number + n * t;
					t /= 10.0;
				}
				else if(meetE)
				{
					auto pair = ParseInteger(str.substr(i));
					if(!std::get<1>(pair))
					{
						success = false;
						break;
					}
					Float64 e = std::get<0>(pair);
					number = number * pow(10.0, e);
					// fast break
					i = str.size() - 1;
				}
				else
				{
					number = number * 10 + n;
				}
			}
			else if(c == '.')
			{
				if(meetDot || meetE)
				{
					success = false;
					break;
				}
				meetDot = true;
			}
			else if(c == 'e' || c == 'E')
			{
				if(meetE)
				{
					success = false;
					break;
				}
				meetE = true;
				meetDot = false;
			}
			else if(c == 'f' && i != str.size() - 1)
			{
				success = false;
				break;
			}
		}
	}

	return std::make_tuple(number * sign, success);
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