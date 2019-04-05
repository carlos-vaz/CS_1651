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
label:	while(1) {
		printf("Hello from thread 1! My arg is %ld\n", (long)arg);
		pet_thread_yield_to(test_thread2);
	}
	return NULL;
}

void *
test_func2(void * arg)
{
	while(1) {
		printf("Hello from thread 2! My arg is %ld\n", (long)arg);
		pet_thread_yield_to(test_thread1);
	}
	return NULL;
}


int main(int argc, char ** argv)
{
    int ret = 0;

    ret = pet_thread_init();

    if (ret == -1) {
	ERROR("Could not initialize Pet Thread Library\n");
	return -1;
    }

    
    printf("Testing Pet Thread Library\n");


    ret = pet_thread_create(&test_thread1, test_func1, (void *)1);
    ret = pet_thread_create(&test_thread2, test_func2, (void *)2);

    if (ret == -1) {
	ERROR("Could not create test_thread1\n");
	return -1;
    }
    


    ret = pet_thread_run();

    if (ret == -1) {
	ERROR("Error encountered while running pet threads (ret=%d)\n", ret);
	return -1;
    }
    
    return 0;

}
