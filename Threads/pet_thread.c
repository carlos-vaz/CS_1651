/* Pet Thread Library
 *  (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 */


#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "pet_thread.h"
#include "pet_hashtable.h"
#include "pet_list.h"
#include "pet_log.h"


#define STACK_SIZE (4096 * 32)




typedef enum {PET_THREAD_STOPPED,
	      PET_THREAD_RUNNING,
 	      PET_THREAD_READY,
	      PET_THREAD_BLOCKED} thread_run_state_t;

struct exec_ctx {
    uint64_t rbp;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rip;
} __attribute__((packed));


struct pet_thread {
    /* Implement this */
};



static pet_thread_id_t current     = PET_MASTER_THREAD_ID;
struct pet_thread      master_dummy_thread;

static LIST_HEAD(thread_list);


extern void __switch_to_stack(void            * tgt_stack,
			      void            * saved_stack,
			      pet_thread_id_t   current,
			      pet_thread_id_t   tgt);

extern void __abort_to_stack(void * tgt_stack);

static struct pet_thread *
get_thread(pet_thread_id_t thread_id)
{

    if (thread_id == PET_MASTER_THREAD_ID) {
	return &(master_dummy_thread);
    }


    /* Implement this */
    
    return NULL;
}

static pet_thread_id_t
get_thread_id(struct pet_thread * thread)
{
    if (thread == &(master_dummy_thread)) {
	return PET_MASTER_THREAD_ID;
    }

    /* Implement this */
    
    return 0;
}


int
pet_thread_init(void)
{
    printf("Initializing Pet Thread library\n");

    /* Implement this */
    
    return 0;
}


static void
__dump_stack(struct pet_thread * thread)
{

    /* Implement this */
    
    return;
}



int
pet_thread_join(pet_thread_id_t    thread_id,
		void            ** ret_val)
{

    /* Implement this */
    
    return 0;
}


void
pet_thread_exit(void * ret_val)
{
    /* Implement this */
}



static int
__thread_invoker(struct pet_thread * thread)
{

    /* Implement this */
    
    return 0;
    
}


int
pet_thread_create(pet_thread_id_t * thread_id,
		  pet_thread_fn_t   func,
		  void            * arg)
{
    struct pet_thread * new_thread;
    
    /* Implement this */
    
    DEBUG("Created new Pet Thread (%p):\n", new_thread);
    DEBUG("--Add thread state here--\n");
    __dump_stack(new_thread);

    
    return 0;
}


void
pet_thread_cleanup(pet_thread_id_t prev_id,
		   pet_thread_id_t my_id)
{
    /* Implement this */
    
}




static void
__yield_to(struct pet_thread * tgt_thread)
{
    /* Implement this */
}


int
pet_thread_yield_to(pet_thread_id_t thread_id)
{
    __yield_to(get_thread(thread_id));

    return 0;
}





int
pet_thread_schedule()
{

    /* Implement this */
    
    return 0;
}






int
pet_thread_run()
{

    printf("Starting Pet Thread execution\n");
    
    
    pet_thread_schedule();
    
    printf("Pet Thread execution has finished\n");

    return 0;
}
	     
