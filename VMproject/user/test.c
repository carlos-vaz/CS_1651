#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "harness.h"


int main(int argc, char ** argv) {
	init_petmem();

	char * buf, *buf1, *buf2, *buf3, *buf4, *buf5;
	buf = pet_malloc(4096);
	pet_dump();
	buf1 = pet_malloc(8192);
	pet_dump();
	buf2 = pet_malloc(2176353);
	pet_dump();
	buf3 = pet_malloc(4096);
	pet_dump();
	buf4 = pet_malloc(40960);
	pet_dump();
	buf5 = pet_malloc(234);
	pet_dump();

	//printf("Allocated 1 page at %p\n", buf);


	buf[50] = 'H';
	buf[51] = 'e';
	buf[52] = 'l';
	buf[53] = 'l';
	buf[54] = 'o';
	buf[55] = ' ';
	buf[56] = 'W';
	buf[57] = 'o';
	buf[58] = 'r';
	buf[59] = 'l';
	buf[60] = 'd';
	buf[61] = '!';
	buf[62] = 0;


	printf("%s\n", (char *)(buf + 50));



	pet_free(buf);
	pet_dump();
	pet_free(buf1);
	pet_dump();
	pet_free(buf2);
	pet_dump();
	pet_free(buf3);
	pet_dump();
	pet_free(buf4);
	pet_dump();
	pet_free(buf5);
	pet_dump();

	char * b = malloc(20);
	b[0] = 'u';
	b[20] = 'f'; // Will cause segfault (lack of permission?)

	/*
	printf("%s\n", (char *)(buf + 50));
	*/ 

	return 0;

}
