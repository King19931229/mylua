#include <cstdio>
#include <vector>
#include <memory>
#include <cstring>
#include "binchunk/binary_chunk.h"
#include "binchunk/reader.h"

int main()
{
	/*
	FILE* f = fopen("C:/LearnCompiler/lua-5.3.6/src/hello.luac", "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<char> buffer;
		buffer.resize(size + 1);
		buffer[size] = 0;

		fread(buffer.data(), 1, size, f);
		fclose(f);
		f = NULL;

		Reader reader{buffer};
		reader.CheckHeader();
	}
	*/
	char test0 = 230;
	unsigned char test1 = 230;
	// transfer to int compare
	if(test0 == test1)
	{
		printf("ok\n");
	}
	if((char)test0 == (char)test1)
	{
		printf("signed ok\n");
	}
	if((unsigned char)test0 == (unsigned char)test1)
	{
		printf("unsigned ok\n");
	}
}
