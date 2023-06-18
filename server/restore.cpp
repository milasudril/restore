//@	{"target":{"name":"restore.o"}}

#include "./config.hpp"

int main(int argc, char**)
{
	if(argc < 2)
	{
		fprintf(stderr, "Config file not specified\n");
		return -1;
	}

	return 0;
}