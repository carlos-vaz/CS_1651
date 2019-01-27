#include <stdio.h>
#include "locking.h"

#define N 1000

void without_mem_barrier() {
	int i = 0;
	int a[N];
	for (; i<N; i++)
		a[i]++;
}

void with_mem_barrier() {
	int i=0;
	int a[N];
	for (; i<N; i++) {
		//asm (""::"m" (i));
		asm ("":::"memory");
		a[i]++;
	}
}

int main() {
	// Memory barriers
	without_mem_barrier();
	with_mem_barrier();

	// Atomic add/sub
	int i=7; int j=10;
	atomic_add(&i,j);
	printf("i <-- i + j = %d\n", i);
	atomic_sub(&j,i);
	printf("j <-- j - i = %d\n", j);

	// Atomic compare&swap
	unsigned int a = 9; unsigned int b = 10; unsigned int c = 9; unsigned int d = 17;
	printf("C&S a_old = %d", compare_and_swap(&a, &b, 17));
	printf(" a_new = %d\n", a);
	printf("C&S a_old = %d", compare_and_swap(&a, &c, 17));
	printf(" a_new = %d\n", a);
	printf("C&S a_old = %d", compare_and_swap(&a, &d, 23));
	printf(" a_new = %d\n", a);
	unsigned int free = 1; unsigned int expected = 1;
	compare_and_swap(&free, &expected, 0);
	printf("free = %d, expected = %d\n", free, expected);
	compare_and_swap(&free, &expected, 0);
	printf("free = %d, expected = %d\n", free, expected);
}




