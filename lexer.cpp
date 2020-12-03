#include "lexer/lexer.h"

#ifdef Error
#	undef Error
#endif

#define Error(...)\
{\
	String msg = Format::FormatString(__VA_ARGS__);\
	msg = Format::FormatString("%s:%d %s", chunkName.c_str(), line, msg.c_str());\
	panic(msg.c_str());\
}

Lexer::Lexer(const String& _chunk, const String& _chunkName, int _line)
{
	chunk = _chunk;
	chunkName = _chunkName;
	line = _line;
	peekIndex = 0;

	nextToken = "";
	nextTokenKind = 0;
	nextTokenLine = 0;

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

TokenResult Lexer::NextToken()
{
	if(nextTokenLine > 0)
	{
		TokenResult res = {nextTokenLine, nextTokenKind, nextToken};
		line = nextTokenLine;
		nextTokenLine = 0;
		return res;
	}

	SkipWhiteSpaces();
	if(peekIndex == chunk.length())
	{
		return {line, TOKEN_EOF, "EOF"};
	}

	switch (chunk[peekIndex])
	{
		case ';': Next(1); return {line, TOKEN_SEP_SEMI, ";"};
		case ',': Next(1); return {line, TOKEN_SEP_COMMA, ","};
		case '(': Next(1); return {line, TOKEN_SEP_LPAREN, "("};
		case ')': Next(1); return {line, TOKEN_SEP_RPAREN, ")"};
		case ']': Next(1); return {line, TOKEN_SEP_RBRACK, "]"};
		case '{': Next(1); return {line, TOKEN_SEP_LCURLY, "{"};
		case '}': Next(1); return {line, TOKEN_SEP_RCURLY, "}"};
		case '+': Next(1); return {line, TOKEN_OP_ADD, "+"};
		case '-': Next(1); return {line, TOKEN_OP_MINUS, "-"};
		case '*': Next(1); return {line, TOKEN_OP_MUL, "*"};
		case '^': Next(1); return {line, TOKEN_OP_POW, "^"};
		case '%': Next(1); return {line, TOKEN_OP_MOD, "%"};
		case '&': Next(1); return {line, TOKEN_OP_BAND, "&"};
		case '|': Next(1); return {line, TOKEN_OP_BOR, "|"};
		case '#': Next(1); return {line, TOKEN_OP_LEN, "#"};
		case ':':
		{
			if(Test("::"))
			{
				Next(2); return {line, TOKEN_SEP_LABEL, "::"};
			}
			else
			{
				Next(1); return {line, TOKEN_SEP_COLON, ":"};
			}
		}
		case '/':
		{
			if(Test("//"))
			{
				Next(2); return {line, TOKEN_OP_IDIV, "//"};
			}
			else
			{
				Next(1); return {line, TOKEN_OP_DIV, "/"};
			}
		}
		case '~':
		{
			if(Test("~="))
			{
				Next(2); return {line, TOKEN_OP_NE, "~="};
			}
			else
			{
				Next(1); return {line, TOKEN_OP_WAVE, "~"};
			}
		}
		case '=':
		{
			if(Test("=="))
			{
				Next(2); return {line, TOKEN_OP_EQ, "=="};
			}
			else
			{
				Next(1); return {line, TOKEN_OP_ASSIGN, "="};
			}
		}
		case '<':
		{
			if(Test("<="))
			{
				Next(2); return {line, TOKEN_OP_LE, "<="};
			}
			else if(Test("<<"))
			{
				Next(2); return {line, TOKEN_OP_SHL, "<<"};
			}
			else
			{
				Next(1); return {line, TOKEN_OP_LT, "<"};
			}
		}
		case '>':
		{
			if(Test(">="))
			{
				Next(2); return {line, TOKEN_OP_GE, ">="};
			}
			else if(Test(">>"))
			{
				Next(2); return {line, TOKEN_OP_SHR, ">>"};
			}
			else
			{
				Next(1); return {line, TOKEN_OP_GT, ">"};
			}
		}
		case '.':
		{
			if(Test("..."))
			{
				Next(3); return {line, TOKEN_VARARG, "..."};
			}
			else if(Test(".."))
			{
				Next(2); return {line, TOKEN_OP_CONCAT, ".."};
			}
			else if(peekIndex == chunk.length() - 1 || !IsDigit(chunk[peekIndex + 1]))
			{
				Next(1); return {line, TOKEN_SEP_DOT, "."};
			}
		}
		case '[':
		{
			if(Test("[[") || Test("[="))
			{
				return {line, TOKEN_STRING, ScanLongString()};
			}
			else
			{
				Next(1); return {line, TOKEN_SEP_LBRACK, "["};
			}
		}
		case '\'':
		case '\"':
		{
			return {line, TOKEN_STRING, ScanShortString()};
		}
	}

	Byte c = chunk[peekIndex];
	if(c == '.' || IsDigit(c))
	{
		return {line, TOKEN_NUMBER, ScanNumber()};
	}
	if(c == '_' || IsLetter(c))
	{
		String token = ScanIdentifier();
		auto it = keywords.find(token);
		if(it != keywords.end())
		{
			TokenKind kind = TokenKind(it->second);
			return {line, kind, token};
		}
		else
		{
			return {line, TOKEN_IDENTIFIER, token};
		}
	}

	Error("unreachable!");
	return {line, TOKEN_EOF, "EOF"};
}

TokenKindResult Lexer::NextTokenKind(int kind)
{
	TokenResult res = NextToken();
	if(res.kind != kind)
	{
		Error("syntax error near '%s'", res.token);
	}
	return {res.line, res.token};
}

TokenKindResult Lexer::NextIdentifier()
{
	return NextTokenKind(TOKEN_IDENTIFIER);
}

int Lexer::Line()
{
	return line;
}

int Lexer::LookAhead()
{
	if(nextTokenLine > 0)
	{
		return nextTokenKind;
	}
	int currentLine = line;
	TokenResult res = NextToken();
	line = currentLine;
	nextTokenLine = res.line;
	nextTokenKind = res.kind;
	nextToken = res.token;
	return nextTokenKind;
}

void Lexer::SkipWhiteSpaces()
{
	while(peekIndex < chunk.length())
	{
		if(Test("--"))
		{
			SkipComment();
		}
		else if(Test("\r\n") || Test("\n\r"))
		{
			Next(2);
			line += 1;
		}
		else if(IsNewLine(chunk[peekIndex]))
		{
			Next(1);
			line += 1;
		}
		else if(IsWhileSpace(chunk[peekIndex]))
		{
			Next(1);
		}
		else
		{
			break;
		}
	}
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

bool Lexer::Peek(Byte c)
{
	if(!IsFinish())
	{
		return chunk[peekIndex] == c;
	}
	return false;
}

bool Lexer::IsDigit(Byte c)
{
	return c >= '0' && c <= '9';
}

bool Lexer::IsHex(Byte c)
{
	return (c >= '0' && c <= '9') ||  (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool Lexer::IsLetter(Byte c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::IsFinish()
{
	return peekIndex < chunk.length();
}

void Lexer::SkipComment()
{
	Next(2);
	if(Peek('['))
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

	if(!Peek('['))
		Error("invalid long string missing [");
	Next(1);

	while(Peek('='))
	{
		Next(1);
		++leftEqualCount;
	}

	if(!Peek('['))
		Error("invalid long string missing [");
	Next(1);

	start = peekIndex;
	while(!Peek(']') && !IsFinish())
		Next(1);

	if(!Peek(']'))
		Error("invalid long string missing ]");

	end = peekIndex;

	while(Peek('='))
	{
		Next(1);
		++rightEqualCount;
	}

	if(!Peek(']'))
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

	if(Peek('\''))
	{
		brack = '\'';
		Next(1);
	}
	else if(Peek('\"'))
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
		// Scan \' and \" first in case end by mistake.
		if(Test("\\\""))
		{
			Next(2);
		}
		if(Test("\\\'"))
		{
			Next(2);
		}
		if(Peek(brack))
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

String Lexer::ScanNumber()
{
	if(!IsFinish())
	{
		size_t beg = 0, end = 0;
		// 0[xX]
		if(Test("0x") || Test("0X"))
		{
			Next(2);
			beg = peekIndex;
			// 0[xX][0-9a-fA-F]*
			while (!IsFinish() && IsHex(chunk[peekIndex]))
				Next(1);
			// 0[xX][0-9a-fA-F]*(\.)?
			if(Peek('.'))
			{
				Next(1);
				// 0[xX][0-9a-fA-F]*(\.[0-9a-fA-F]*)?
				while (!IsFinish() && IsHex(chunk[peekIndex]))
					Next(1);
			}
			// 0[xX][0-9a-fA-F]*(\.[0-9a-fA-F]*)?([pP])?
			if(Peek('p') || Peek('P'))
			{
				Next(1);
				// 0[xX][0-9a-fA-F]*(\.[0-9a-fA-F]*)?([pP][+\-]?)?
				if(Peek('+') || Peek('-'))
					Next(1);
				// 0[xX][0-9a-fA-F]*(\.[0-9a-fA-F]*)?([pP][+\-]?[0-9]+)?
				if(!IsFinish() && IsDigit(chunk[peekIndex]))
					Next(1);
				else
					Error("float hex number has no digit after p or P");
				while(!IsFinish() && IsDigit(chunk[peekIndex]))
					Next(1);
			}
			end = peekIndex;
			return chunk.substr(beg, end - beg);
		}
		else if(IsDigit(chunk[peekIndex]))
		{
			beg = peekIndex;
			// [0-9]*
			while(!IsFinish() && IsDigit(chunk[peekIndex]))
				Next(1);
			// [0-9]*(\.)?
			if(Peek('.'))
			{
				Next(1);
				// [0-9]*(\.[0-9]*)?
				while(!IsFinish() && IsDigit(chunk[peekIndex]))
					Next(1);
			}
			// [0-9]*(\.[0-9]*)?([eE])?
			if(Peek('e') || Peek('E'))
			{
				Next(1);
				// [0-9]*(\.[0-9]*)?([eE][+\-]?)?
				if(Peek('+') || Peek('-'))
					Next(1);
				// [0-9]*(\.[0-9]*)?([eE][+\-]?[0-9]+)?
				if(!IsFinish() && IsDigit(chunk[peekIndex]))
					Next(1);
				else
					Error("float number has no digit after e or E");
				while(!IsFinish() && IsDigit(chunk[peekIndex]))
					Next(1);
			}
			end = peekIndex;
			return chunk.substr(beg, end - beg);
		}
	}
	Error("unreachable!");
	return "0";
}

String Lexer::ScanIdentifier()
{
	if(!IsFinish())
	{
		if(Peek('_') || IsLetter(chunk[peekIndex]))
		{
			size_t beg = peekIndex, end = peekIndex;
			while(Peek('_'))
				Next(1);
			while(!IsFinish() &&
					(IsLetter(chunk[peekIndex]) ||
					IsDigit(chunk[peekIndex]) ||
					Peek('_')))
				Next(1);
			end = peekIndex;
			return chunk.substr(beg, end - beg);
		}
	}
	Error("unreachable!");
	return "";
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
					for(Byte c = str[temp]; c >= '0' && c <= '9';)
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
						res += (Byte)dec;
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
					for(Byte c = str[temp]; (c >= '0' && c <= '9') ||
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
						Byte c = str[temp];
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
						res += (Byte)hex;
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
							Byte c = str[temp];
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
								res += (Byte)unicode;
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