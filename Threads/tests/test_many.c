#include <stdio.h>
#include <stdlib.h>


#include "../pet_thread.h"
#include "../pet_log.h"


#define NUM_THREADS 20
#define NUM_CYCLES 2

pet_thread_id_t test_thread[NUM_THREADS];

void *
test_yield(void * thread_num)
{
	int i;
	for(i=0; i<NUM_CYCLES; i++){
		printf("%*c|%d|\n", (int)(long)thread_num+1,' ', (int)(long)thread_num+1);                        
		
		if((long)thread_num < NUM_THREADS-1)
		    pet_thread_yield_to(test_thread[(long)thread_num+1]);

		printf("%*c|%d|\n", (int)(long)thread_num+1,' ', (int)(long)thread_num+1);
		
		if((long)thread_num > 0)
		    pet_thread_yield_to(test_thread[(long)thread_num-1]);
	}	
    
        return NULL; 
}


void *
test_nested_join(void * thread_num)
{

       void * ret_val;

       printf("%*c|%d|\n", (int)(long)thread_num+1,' ', (int)(long)thread_num+1);                       

       if((long)thread_num < NUM_THREADS-1)
           pet_thread_join(test_thread[(long)thread_num+1], &ret_val);

       printf("%*c|%d|\n", (int)(long)thread_num+1,' ', (int)(long)thread_num+1);
      
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

   	
    int i;
    for(i = 0; i<NUM_THREADS; i++){
	
    	ret = pet_thread_create(&test_thread[i], test_yield, (void *)(uintptr_t)i);

    	if (ret == -1) {
		ERROR("Could not create test_thread %d\n", i);
		return -1;
    	}
    }

    printf("\n\nnote: for easy graphical checking set VERBOSE 0 in pet_thread.c\n\n");

    printf("Testing pet_thread_yield_to(...) with %d threads & %d cycles\n\n", NUM_THREADS, NUM_CYCLES);

    ret = pet_thread_run();

    if (ret == -1) {
        ERROR("Error encountered while running pet threads (ret=%d)\n", ret);
        return -1;
    }




    //test nested joins many

    for(i = 0; i<NUM_THREADS; i++){

        ret = pet_thread_create(&test_thread[i], test_nested_join, (void *)(uintptr_t)i);

        if (ret == -1) {
                ERROR("Could not create test_thread %d\n", i);
                return -1;
        }
    }


    printf("\n\nTesting pet_thread_join(...) nested joins with %d threads (1 cycle only since join waits for thread to exit)\n\n", NUM_THREADS);

    ret = pet_thread_run();

    if (ret == -1) {
        ERROR("Error encountered while running pet threads (ret=%d)\n", ret);
        return -1;
    }










    return 0;

}
