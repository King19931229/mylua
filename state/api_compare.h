#pragma once
#include "api/consts.h"
#include "state/lua_value.h"

inline bool _eq(const LuaValue& a, const LuaValue& b)
{
	switch (a.tag)
	{
		case LUA_TNIL: return b.tag == LUA_TNIL;
		case LUA_TBOOLEAN: return b.tag == LUA_TBOOLEAN && a.boolean == b.boolean;
		case LUA_TSTRING: return b.tag == LUA_TSTRING && a.str == b.str;
		case LUA_TNUMBER:
		{
			if(b.tag == LUA_TNUMBER)
			{
				if(a.isfloat)
				{
					if(b.isfloat)
						return a.number == b.number;
					else
						return a.number == (Float64)b.integer;
				}
				else
				{
					if(b.isfloat)
						return (Float64)a.integer == b.number;
					else
						return a.integer == b.integer;
				}
			}
			return false;
		}
		default: return a == b;
	}
}

inline bool _lt(const LuaValue& a, const LuaValue& b)
{
	switch (a.tag)
	{
		case LUA_TSTRING: if(b.tag == LUA_TSTRING) return a.str <= b.str; break;
		case LUA_TNUMBER:
		{
			if(b.tag == LUA_TNUMBER)
			{
				if(a.isfloat)
				{
					if(b.isfloat)
						return a.number <= b.number;
					else
						return a.number <= (Float64)b.integer;
				}
				else
				{
					if(b.isfloat)
						return (Float64)a.integer <= b.number;
					else
						return a.integer <= b.integer;
				}
			}
			break;
		}
		default: break;
	}
	panic("comparsion error!");
	return false;
}

inline bool _le(const LuaValue& a, const LuaValue& b)
{
	switch (a.tag)
	{
		case LUA_TSTRING: if(b.tag == LUA_TSTRING) return a.str < b.str; break;
		case LUA_TNUMBER:
		{
			if(b.tag == LUA_TNUMBER)
			{
				if(a.isfloat)
				{
					if(b.isfloat)
						return a.number < b.number;
					else
						return a.number < (Float64)b.integer;
				}
				else
				{
					if(b.isfloat)
						return (Float64)a.integer < b.number;
					else
						return a.integer < b.integer;
				}
			}
			break;
		}
		default: break;
	}
	panic("comparsion error!");
	return false;
}