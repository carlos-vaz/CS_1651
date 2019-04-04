/* Pet Thread Library
 *  (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 */


#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

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
    uint64_t rip;	// pushed by "call" instruction into __switch_to_stack()
} __attribute__((packed));


struct pet_thread {
	pet_thread_id_t id;
	pet_thread_fn_t func;
	void * stack;
	void * stack_rsp;
	struct list_head list;
	thread_run_state_t state;
};


static int thread_id_count = 1;

static pet_thread_id_t current     = PET_MASTER_THREAD_ID;
struct pet_thread      master_dummy_thread;

static LIST_HEAD(ready_list);
static LIST_HEAD(blocked_list);
static struct pet_thread 	*running;


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
	
	if(running->id==thread_id)
		return running;
	
	struct pet_thread * pos;
	list_for_each_entry(pos, &ready_list, list) {
		if(pos->id==thread_id) {
			return pos;
		}
	}
	list_for_each_entry(pos, &blocked_list, list) {
		if(pos->id==thread_id) {
			return pos;
		}
	}

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

	list_head_init(&ready_list);
	list_head_init(&blocked_list);

	master_dummy_thread.state = PET_THREAD_RUNNING;
	master_dummy_thread.id = PET_MASTER_THREAD_ID;
	running = &master_dummy_thread;

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
	assert(thread->state==PET_THREAD_READY);
	assert(running->state==PET_THREAD_RUNNING);
	struct pet_thread * old_thread = running;
	running = thread;

	// TODO: Route old_thread to blocked queue if necessary
	old_thread->state = PET_THREAD_READY;
	if(old_thread != &master_dummy_thread)
		list_add_tail(&old_thread->list, &ready_list);

	running->state = PET_THREAD_RUNNING;
	current = thread->id;
	__switch_to_stack(thread->stack_rsp, old_thread->stack_rsp, thread->id, old_thread->id);
	return 0;
}


int
pet_thread_create(pet_thread_id_t * thread_id,
		  pet_thread_fn_t   func,
		  void            * arg)
{
	// Create thread struct
	struct pet_thread * new_thread = (struct pet_thread*)malloc(sizeof(struct pet_thread));
	new_thread->id = (pet_thread_id_t)thread_id_count++;
	*thread_id = new_thread->id;
	new_thread->func = func;
	new_thread->state = PET_THREAD_READY;

	// Allocate stack
	void * stack = calloc(STACK_SIZE, 1);
	new_thread->stack = stack;

	// Add to ready_list
	list_add_tail(&new_thread->list, &ready_list);

	DEBUG("Created new Pet Thread (%p) with id %d:\n", new_thread, (int)new_thread->id);
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



/* 
 * Puts running thread in running queue, 
 */
static void
__yield_to(struct pet_thread * tgt_thread)
{
	if(tgt_thread->state != PET_THREAD_READY) {
		printf("WARNING: Thread yielded to is not ready. Scheduling another\n");
		pet_thread_schedule();
		return; // returning point A... Continue with func
	}
	__thread_invoker(tgt_thread);
	// returning point B... Continue with func
}


int
pet_thread_yield_to(pet_thread_id_t thread_id)
{
    __yield_to(get_thread(thread_id));

    return 0;
}


void dump_list(struct list_head *head, char* name) {
	printf("\n-- -- --  --  --  %s  (th=%d) --  --  --  --  --\n", name, thread_id_count);
	struct pet_thread *pos, *n;
	list_for_each_entry_safe(pos, n, head, list) {
		printf("%d ", (int)pos->id);
		if(&n->list!=head)
			printf("--> ");
	}
	printf("\n-- -- --  --  --   --  --  --  --  --  --\n");
}


int
pet_thread_schedule()
{
	while( !list_empty(&ready_list) ) {
		// If ready queue is empty, schedule master_dummy_thread
		if(list_empty(&ready_list) && running!=&master_dummy_thread) {
			__thread_invoker(&master_dummy_thread);
			return 9;	// NEVER REACHED
		}

		dump_list(&ready_list, "Ready List");

		struct pet_thread *pos;
		list_for_each_entry(pos, &ready_list, list) {
			assert(pos->state == PET_THREAD_READY);
			__thread_invoker(pos);
			// Won't get here until master thread is re-invoked
			break;
		}
	}
    
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
	     
