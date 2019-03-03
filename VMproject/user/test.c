#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "harness.h"


int main(int argc, char ** argv) {
	init_petmem();

	char * buf, *buf1, *buf2, *buf3, *buf4, *buf5;
	buf = pet_malloc(4096);
	pet_dump();
/*	buf1 = pet_malloc(8192);
	pet_dump();
	buf2 = pet_malloc(2176353);
	pet_dump();
	buf3 = pet_malloc(4096);
	pet_dump();
	buf4 = pet_malloc(40960);
	pet_dump();
	buf5 = pet_malloc(234);
	pet_dump();
*/
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
/*	pet_free(buf1);
	pet_dump();
	pet_free(buf2);
	pet_dump();
	pet_free(buf3);
	pet_dump();
	pet_free(buf4);
	pet_dump();
	pet_free(buf5);
	pet_dump();
*/
/*	char * b = malloc(20);
	b[0] = 'u';
	b[20] = 'u'; 
	b[4095] = 'u';
	b[4096] = 'f';
	b[8191] = 'f';
	b[8192] = 'f';
	b[20*4096] = 'f';
	printf("b[0] = %c\nb[20] = %c\nb[4095] = %c\nb[4096] = %c\n", b[0], b[20], b[4095], b[4096]);
	printf("b[8191] = %c\nb[8192] = %c\nb[20*4096] = %c\n", b[8191], b[8192], b[20*4096]);
*/
	/*
	printf("%s\n", (char *)(buf + 50));
	*/ 

	return 0;

}
