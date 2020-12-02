#include "lexer/lexer.h"

Lexer::Lexer(const String& _chunk, const String& _chunkName, int _line)
{
	chunk = _chunk;
	chunkName = _chunkName;
	line = _line;
	peekIndex = 0;

	keywords.insert({"and", TOKEN_OP_AND});
	keywords.insert({"break", TOKEN_KW_BREAK});
	keywords.insert({"do", TOKEN_KW_DO});
	keywords.insert({"else", TOKEN_KW_ELSE});
	keywords.insert({"elseif", TOKEN_KW_ELSEIF});
	keywords.insert({"end", TOKEN_KW_END});
	keywords.insert({"false", TOKEN_KW_FALSE});
	keywords.insert({"for", TOKEN_KW_FOR});
	keywords.insert({"function", TOKEN_KW_FUNCTION});
	keywords.insert({"goto", TOKEN_KW_GOTO});
	keywords.insert({"if", TOKEN_KW_IF});
	keywords.insert({"in", TOKEN_KW_IN});
	keywords.insert({"local", TOKEN_KW_LOCAL});
	keywords.insert({"nil", TOKEN_KW_NIL});
	keywords.insert({"not", TOKEN_OP_NOT});
	keywords.insert({"or", TOKEN_OP_OR});
	keywords.insert({"repeat", TOKEN_KW_REPEAT});
	keywords.insert({"return", TOKEN_KW_RETURN});
	keywords.insert({"then", TOKEN_KW_THEN});
	keywords.insert({"true", TOKEN_KW_TRUE});
	keywords.insert({"until", TOKEN_KW_UNTIL});
	keywords.insert({"while", TOKEN_KW_WHILE});	
}

//#define CHECK_OUT_OF_BOUND(index) if(index >= chunk.length()) goto OUT_OUT_BOUND;

void Lexer::NextToken(int& line, int& kind, String& token)
{

}

#ifdef Error
#	undef Error
#endif

#define Error(...)\
{\
	String msg = Format::FormatString(__VA_ARGS__);\
	msg = Format::FormatString("%s:%d %s", chunkName.c_str(), line, msg.c_str());\
	panic(msg.c_str());\
}

bool Lexer::Test(const String& s)
{
	if(peekIndex + s.length() <= chunk.length() &&
		chunk.substr(peekIndex, s.length()) == s)
		return true;
	return false;
}

void Lexer::Next(int n)
{
	if(peekIndex + n <= chunk.length())
		peekIndex += n;
	else
		peekIndex = chunk.length();
}

bool Lexer::IsWhileSpace(Byte c)
{
	switch(c)
	{
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case ' ':
			return true;
		default:
			return false;
	}
}

bool Lexer::IsNewLine(Byte c)
{
	return c == '\r' || c == '\n';
}

bool Lexer::IsCharacter(Byte c)
{
	if(!IsFinish())
	{
		return chunk[peekIndex] == c;
	}
	return false;
}

bool Lexer::IsFinish()
{
	return peekIndex < chunk.length();
}

void Lexer::SkipComment()
{
	Next(2);
	if(IsCharacter('['))
	{
		size_t tempPeekIndex = peekIndex + 1;
		while(tempPeekIndex < chunk.length() && chunk[tempPeekIndex] == '=')
			++tempPeekIndex;
		if(tempPeekIndex < chunk.length() && chunk[tempPeekIndex] == '[')
		{
			ScanLongString();
			return;
		}
	}
	while(!IsFinish() && !IsNewLine(chunk[peekIndex]))
		Next(1);
}

String Lexer::ScanLongString()
{
	size_t leftEqualCount = 0, rightEqualCount = 0;
	size_t start = 0, end = 0;
	String str;

	if(!IsCharacter('['))
		Error("invalid long string missing [");
	Next(1);

	while(IsCharacter('='))
	{
		Next(1);
		++leftEqualCount;
	}

	if(!IsCharacter('['))
		Error("invalid long string missing [");
	Next(1);

	start = peekIndex;
	while(!IsCharacter(']') && !IsFinish())
		Next(1);

	if(!IsCharacter(']'))
		Error("invalid long string missing ]");

	end = peekIndex;

	while(IsCharacter('='))
	{
		Next(1);
		++rightEqualCount;
	}

	if(!IsCharacter(']'))
		Error("invalid long string missing ]");
	Next(1);

	if(leftEqualCount != rightEqualCount)
		Error("invalid long string missing left = count is not equal to right = count");

	str = chunk.substr(start, end - start);
	str = ReplaceLine(str);

	line += Count(str, '\n');

	if(str.length() > 0 && str[0] == '\n')
		str = str.substr(1);
	
	return str;
}

String Lexer::ScanShortString()
{
	Byte brack = 0;
	size_t beg = 0, end = 0;
	bool closed = false;
	String str;

	if(IsCharacter('\''))
	{
		brack = '\'';
		Next(1);
	}
	else if(IsCharacter('\"'))
	{
		brack = '\"';
		Next(1);
	}
	else
	{
		Error("invalid short string missing \' or \"");
	}

	beg = peekIndex;
	while(!IsFinish())
	{
		if(Test("\\\""))
		{
			Next(2);
		}
		if(Test("\\\'"))
		{
			Next(2);
		}
		if(IsCharacter(brack))
		{
			Next(1);
			closed = true;
			break;
		}
		else
		{
			Next(1);
		}
	}
	end = peekIndex;

	if(!closed)
		Error("invalid short string not closed");

	str = chunk.substr(beg, end - beg);
	str = ReplaceLine(str);
	line += Count(str, '\n');
	str = Escape(str);

	return str;
}

String Lexer::Escape(const String& str)
{
	String res;
	res.reserve(str.length());
	for(size_t pos = 0; pos < str.length();)
	{
		if(str[pos] != '\\')
		{
			res += str[pos];
			++pos;
			continue;
		}
		if(pos == str.length() - 1)
		{
			Error("unflished string");
		}

		switch (str[pos + 1])
		{
			case 'a': res += '\a'; pos += 2; continue;
			case 'b': res += '\b'; pos += 2; continue;
			case 'f': res += '\f'; pos += 2; continue;
			case 'n': res += '\n'; pos += 2; continue;
			// very special here \ \n ?
			case '\n': res += '\n'; pos += 2; continue;
			case 'r': res += '\r'; pos += 2; continue;
			case 't': res += '\t'; pos += 2; continue;
			case 'v': res += '\v'; pos += 2; continue;
			case '\"': res += '\"'; pos += 2; continue;
			case '\'': res += '\''; pos += 2; continue;
			case '\\': res += '\\'; pos += 2; continue;

			case '0': case '1': case '2': case '3':	case '4':
			case '5': case '6': case '7': case '8':	case '9':
			{
				if(pos + 1 < str.length())
				{
					size_t temp = pos + 1;
					size_t decCount = 0;
					for(char c = str[temp]; c >= '0' && c <= '9';)
					{
						++decCount;
						++temp;
						if(decCount == 3 ||	temp >= str.length())
							break;
					}

					int dec = 0;
					while(temp >= pos + 1)
					{
						dec = dec * 10 + str[temp] - '0';
						--temp;
					}

					if(temp <= 0xFF)
					{
						res += (unsigned char)dec;
						pos += decCount + 1;
						continue;
					}
					Error("decimal escape too large near %d", temp);
				}
				else
				{
					pos += 1;
				}
				continue;
			}

			case 'x':
			{
				if(pos + 2 < str.length())
				{
					size_t temp = pos + 2;
					size_t hexCount = 0;
					for(char c = str[temp]; (c >= '0' && c <= '9') ||
						(c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');)
					{
						++hexCount;
						++temp;
						if(hexCount == 2 ||	temp >= str.length())
							break;
					}

					int hex = 0;
					while(temp >= pos + 2)
					{
						char c = str[temp];
						if(c >= '0' && c <= '9')
						{
							hex = hex * 16 + c - '0';
						}
						else if(c >= 'a' && c <= 'f')
						{
							hex = hex * 16 + c - 'a';
						}
						else // if(c >= 'A' && c <= 'F')
						{
							hex = hex * 16 + c - 'A';
						}
						--temp;
					}

					if(hexCount >= 1)
					{
						res += (unsigned char)hex;
						pos += hexCount + 2;
						continue;
					}
				}
				pos += 2;
				continue;
			}

			case 'u':
			{
				if(pos + 2 < str.length())
				{
					if(str[pos + 2] == '{')
					{
						size_t temp = pos + 3;
						size_t unicodeCount = 0;
						while(temp < str.length())
						{
							if(str[temp++] == '}')
							{
								break;
							}
							++unicodeCount;
						}

						int unicode = 0;
						--temp;
						while(temp >= pos + 3)
						{
							char c = str[temp];
							if((c >= '0' && c <= '9') ||
								(c >= 'a' && c <= 'f') ||
								(c >= 'A' && c <= 'F'))
							{
								if(c >= '0' && c <= '9')
								{
									unicode = unicode * 16 + c - '0';
								}
								else if(c >= 'a' && c <= 'f')
								{
									unicode = unicode * 16 + c - 'a';
								}
								else // if(c >= 'A' && c <= 'F')
								{
									unicode = unicode * 16 + c - 'A';
								}
							}
							else
							{
								unicodeCount = 0;
								break;
							}
						}

						if(unicodeCount >= 1)
						{
							if(unicode <= 0x10FFFF)
							{
								// TODO
								res += (unsigned char)unicode;
								pos += unicodeCount + 4;
								continue;
							}
							else
							{
								Error("UTF-8 value too large near %d", unicode);
							}
						}
					}
				}	
				pos += 2;
				continue;
			}

			case 'z':
			{
				pos += 2;
				while(pos < str.length() && IsWhileSpace(str[pos]))
					++pos;
				continue;
			}

			default:
			{
				Error("invalid escape sequence near '\\%c'", str[pos + 1]);
			}
		}
	}

	return res;
}

String Lexer::ReplaceLine(const String& str)
{
	String res = str;

	const char* lines[] = {"\r\n", "\n\r", "\r"};

	for(const char* line : lines)
	{
		size_t len = strlen(line);
		size_t pos = res.find(line);
		while(pos != String::npos)
		{
			res.replace(pos, len, "\n");
			pos = res.find(line);
		}
	}

	return res;
}

size_t Lexer::Count(const String& str, Byte c)
{
	size_t count = 0;
	for(size_t i = 0; i < str.length(); ++i)
	{
		if(str[i] == c)
			++count;
	}
	return count;
}

#undef Error