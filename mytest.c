#include <stdio.h>
#include <pthread.h>
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
	printf("\nAtomic add/sub\n");
	int i=7; int j=10;
	atomic_add(&i,j);
	printf("i <-- i + j = %d\n", i);
	atomic_sub(&j,i);
	printf("j <-- j - i = %d\n", j);

	// Atomic compare&swap 32 bit
	printf("\nAtomic compare&swap 32bit\n");
	unsigned int a = 9; 
	printf("C&S a_old = %d", compare_and_swap(&a, 10, 17));
	printf(", a_new = %d\n", a);
	printf("C&S a_old = %d", compare_and_swap(&a, 9, 17));
	printf(", a_new = %d\n", a);
	printf("C&S a_old = %d", compare_and_swap(&a, 17, 23));
	printf(", a_new = %d\n", a);
	unsigned int free = 1; 
	printf("free_old = %d" , compare_and_swap(&free, 1, 0));
	printf(", free = %d\n", free);
	printf("free_old = %d" , compare_and_swap(&free, 1, 0));
	printf(", free = %d\n", free);


	// Atomic compare&swap 64 bit
	printf("\nAtomic compare&swap 64bit\n");
	uintptr_t u, v;
	u = 8; v = 9;
	printf("C&S64 u_old = %ld", compare_and_swap_ptr(&u, 9, 171));
	printf(" u_new = %ld\n", u);
	printf("C&S64 v_old = %ld", compare_and_swap_ptr(&v, 9, 171));
	printf(" v_new = %ld\n", v);

	// Atomic add_ret_prev
	printf("\nAtomic add_ret_prev\n");
	int x=10;
	printf("(add_ret_prev) prev x = %d", atomic_add_ret_prev(&x, 1));
	printf(", new x = %d\n", x);

	// reader writer lock
	

}




