/* Pet Thread Library test driver
 *  (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 */

#include <stdio.h>
#include <stdlib.h>


#include "../pet_thread.h"
#include "../pet_log.h"



pet_thread_id_t test_thread1, test_thread2;

void *
test_func1(void * arg)
{
	void * retval;
test_func1_label:	for(int i=0; i<10; i++) {
		printf("PROG: Hello from thread 1! arg = %ld, i = %d\n", (long)arg, i);
		if(i==5)
			pet_thread_join(test_thread2, &retval);
	}
	//pet_thread_exit(NULL);
	return NULL;
}

void *
test_func2(void * arg)
{
	for(int i=0; i<10; i++) {
		printf("PROG: Hello from thread 2! arg = %ld, i = %d\n", (long)arg, i);
	}
	//pet_thread_exit(NULL);
	return NULL;
}


int main(int argc, char ** argv)
{
    int ret = 0;

    ret = pet_thread_init();

    if (ret == -1) {
	ERROR("PROG: Could not initialize Pet Thread Library\n");
	return -1;
    }

    
    printf("PROG: Testing Pet Thread Library\n");


    ret = pet_thread_create(&test_thread1, test_func1, (void *)10);
    ret = pet_thread_create(&test_thread2, test_func2, (void *)20);

    if (ret == -1) {
	ERROR("PROG: Could not create test_thread1\n");
	return -1;
    }
    


    ret = pet_thread_run();

    if (ret == -1) {
	ERROR("PROG: Error encountered while running pet threads (ret=%d)\n", ret);
	return -1;
    }
    
    return 0;

}
