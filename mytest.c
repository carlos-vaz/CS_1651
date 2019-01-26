//#include <stdio.h>

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
	without_mem_barrier();
	with_mem_barrier();
}
