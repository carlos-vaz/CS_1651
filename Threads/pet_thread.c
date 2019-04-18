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
	pet_thread_fn_t func;			// Debugging only
	void * stack_bottom;			// Used only to free stack
	void * stack_rsp;			// Stack pointer
	struct list_head list;			// LIST_HEAD to read_list & blocked_list
	thread_run_state_t state;
	struct list_head waiting_for_me_list; 	// LIST_HEAD to chain of `waiting_thread'
	uint64_t return_reciever;			// threads calling join() expect a return value here
};

// Chain of threads blocked waiting for another to exit ()
struct waiting_thread {
	struct pet_thread * thread;
	struct list_head list;
};


static int thread_id_count = 1;

static pet_thread_id_t current     = PET_MASTER_THREAD_ID;
struct pet_thread      master_dummy_thread;

static LIST_HEAD(ready_list);
static LIST_HEAD(blocked_list);

static struct pet_thread 	*running;
static struct pet_thread	*stopped;	// Points to the last stopped thread
						// Used to find thread for cleaning


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
	if(stopped!=NULL && stopped->id==thread_id)
		return stopped;
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

	if(thread!=NULL)
		return thread->id;

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


void dump_list(struct list_head *head, char* name) {
	printf("\n-- -- --  --  --  %s  (thds=%d) --  --  --  --  --\n", name, thread_id_count-1);
	struct pet_thread *pos, *n;
	list_for_each_entry_safe(pos, n, head, list) {
		printf("%d ", (int)pos->id);
		if(&n->list!=head)
			printf("--> ");
	}
	printf("\n-- -- --  --  --   --  --  --  --  --  --\n");
}

void dump_list_w(struct list_head *head, char* name) {
	printf("\n-- -- --  --  --  %s  --  --  --  --  --\n", name);
	DEBUG("HEAD thread id: %d\n", (int)list_entry(head, struct pet_thread, waiting_for_me_list)->id);
	struct waiting_thread *pos;
	list_for_each_entry(pos, head, list) {
		printf("%d ", (int)pos->thread->id);
		if(pos->list.next!=head)
			printf("--> ");
	}
	printf("\n-- -- --  --  --   --  --  --  --  --  --\n");
}


/*
 * Returning point for all thread functions that return. Actions: 
 * 
 * 	1. Moves %rax (return) register to a local variable (ret_val) and,
 * 	   if it finds any blocked thread waiting for this exit event, 
 * 	   copies `ret_val' to `return_receiver' of the blocked thread.
 * 
 * 	2. Checks `waiting_for_me_list' of exiting thread to see if any blocked
 * 	   threads can be put in ready_list and scheduled. 
 */
void
__quarantine(void) {
//	uint64_t ret_val;
//from_rax:	asm ("movq %%rax, %0;" : "+m" (ret_val) : : "memory");
	
	DEBUG("FROM __quarantine(): Thread %d exited\n", (int)running->id);

	DEBUG("FROM __quarantine(): Checking thread %d's blocked waiters...\n", (int)running->id);
	// Checked dependent (blocked) threads and move them to ready_list
	struct waiting_thread *pos, *n;
	if(!list_empty(&running->waiting_for_me_list)) {
		dump_list_w(&running->waiting_for_me_list, "WAITING_FOR_ME_LIST");
		list_for_each_entry_safe(pos, n, &running->waiting_for_me_list, list) {
			assert(pos->thread->state == PET_THREAD_BLOCKED);
			//pos->thread->return_reciever = ret_val;	// Copy exiting thread's ret val to blocked thread
			pos->thread->state = PET_THREAD_READY;
			list_del(&pos->thread->list); // remove from blocked_list
			list_add_tail(&pos->thread->list, &ready_list); // add to ready list
			list_del(&pos->list);	// remove from waiting_for_me_list
			free(pos);
		}
		assert(list_empty(&running->waiting_for_me_list));
	}
	else 
		DEBUG("Thread %d has no blocked waiters.\n", (int)running->id);

	// Exit thread and schedule a new one
	dump_list(&ready_list, "RL (check 3)");
	assert(running->state==PET_THREAD_RUNNING);
	assert(running != &master_dummy_thread);
	running->state = PET_THREAD_STOPPED;	// signal thread to be cleaned up on next __switch_stack()
	pet_thread_schedule();
	// If you get here, that means an exited thread was re-invoked (error)
	DEBUG("FROM __quarantine(): ERROR! Thread %d, which exited, was re-invoked\n", (int)running->id);
}

static void
__dump_stack(struct pet_thread * thread)
{
	if(thread == &master_dummy_thread) {
		DEBUG("WARNING: Cannot dump stack of master_dummy_thread.\n");
		return;
	}
	if(thread->state == PET_THREAD_STOPPED) {
		DEBUG("WARNING: Cannot dump stack of exited thread (may have been freed).\n");
		return;
	}
	printf("\n-------- STACK DUMP thread %d --------\n", (int)thread->id);
	uintptr_t * stack = (uintptr_t *)thread->stack_bottom;
	uintptr_t * cur;
	for( int i=0; (uintptr_t *)thread->stack_rsp != cur; i++) {
		cur = &stack[STACK_SIZE/sizeof(uintptr_t) - i];
		printf("%p:\t%lx", cur, *cur);
		if(*cur == (uintptr_t)__quarantine)
			printf(" <__quarantine>");
		if(*cur == (uintptr_t)thread->func)
			printf(" <thread_func>");
		if((uintptr_t *)thread->stack_rsp==cur)
			printf("\t<----- rsp");
		if(*(uintptr_t*)thread->stack_rsp == (uintptr_t)cur)
			printf("\t<----- rbp");
		printf("\n");
	}
	printf("\n");

	return;
}


/*
 * Thread (A) that calls join() will be marked `BLOCKED' (and placed in blocked_list later by __thread_invoker).
 * Then, thread (A) is added to the linked list `waiting_for_me_list' of thread (B) that was 'joined' to.  
 * When thread B exits and executes __quarantine(), its `waiting_for_me_list' field is checked to see if
 * any threads are blocked waiting for it to exit. If so, that those blocked threads are put in the ready_list. 
 */
int
pet_thread_join(pet_thread_id_t    thread_id,
		void            ** ret_val)
{
	DEBUG("ENTERED pet_thread_join\n");
	struct pet_thread *thread = get_thread(thread_id);
	if(thread==NULL) {
		DEBUG("FROM pet_thread_join: Thread to join doesn't exist.\n");
		return -1;
	}
	assert(running->state == PET_THREAD_RUNNING);
	running->state = PET_THREAD_BLOCKED; // __thread_invoker will put thread in blocked_list

	struct waiting_thread * w_t = (struct waiting_thread*)malloc(sizeof(struct waiting_thread));
	w_t->thread = running;
	list_add_tail(&w_t->list, &thread->waiting_for_me_list);
	pet_thread_schedule();

	// Before thread returns to function, fill `ret_val' from this thread's `return_reciever' field
	*ret_val = (void *)running->return_reciever;
	return 0;
}


/*
 * Threads can exit in two ways: 
 *
 * 	1. By calling "ret" x86 instruction, %rip becomes __quarantine(), which 
 *	   immediately checks %rax (return) register and gives the return value
 * 	   to any threads that ask for it. 
 *
 * 	2. By calling this exit() function, the ret_val arg is artificially copied
 *	   to the %rax (return) register. So when __quarantine() is called, the 
 * 	   behavior is the same as 1. 
 */
void
pet_thread_exit(void * ret_val)
{
//to_rax:	asm ("movq %0, %%rax;\n" :  : "m" (ret_val) : "rax");

	// Copy ret_val to return_receiver of threads blocked waiting for this exit event
	DEBUG("Copying ret_val to waiting threads...\n");
	struct waiting_thread *pos, *n;
	if(!list_empty(&running->waiting_for_me_list)) {
		list_for_each_entry_safe(pos, n, &running->waiting_for_me_list, list) {
			assert(pos->thread->state == PET_THREAD_BLOCKED);
			pos->thread->return_reciever = (uint64_t)ret_val;	// Copy exiting thread's ret val to blocked thread
		}
	}

	__quarantine();
}



static int
__thread_invoker(struct pet_thread * thread)
{
	DEBUG("Entered __thread_invoker(id=%d)\n", (int)thread->id);
	assert(thread->state == PET_THREAD_READY);
	assert(running->state != PET_THREAD_READY);
		
	// Move `thread' out of ready_list and into `running'
	struct pet_thread * old_thread = running;
	running = thread;
	dump_list(&ready_list, "RL (check 1)");
	if(thread != &master_dummy_thread) {
		list_del(&thread->list);
		DEBUG("FROM __thread_invoker: Deleting chosen thread from ready_list\n");	
	}
	dump_list(&ready_list, "RL (check 2)");
	

	// Put old_thread in appropriate queue
	switch(old_thread->state) {
		case PET_THREAD_RUNNING	:
			if(old_thread!=&master_dummy_thread) {
				list_add_tail(&old_thread->list, &ready_list);
				DEBUG("FROM __thread_invoker(id=%d)... added old_thread to ready_list\n", (int)thread->id);
			}
			old_thread->state = PET_THREAD_READY;
			break;
		case PET_THREAD_BLOCKED	:
			list_add_tail(&old_thread->list, &blocked_list);
			break;
		case PET_THREAD_STOPPED	:
				stopped = old_thread;
			break;	
	}

	running->state = PET_THREAD_RUNNING;
	current = thread->id;
	DEBUG("FROM __thread_invoker(id=%d), calling __switch_to_stack()\n", (int)thread->id);
	__switch_to_stack(&thread->stack_rsp, &old_thread->stack_rsp, old_thread->id, thread->id);
	return 0;
}


int
pet_thread_create(pet_thread_id_t * thread_id,
		  pet_thread_fn_t   func,
		  void            * arg)
{
	// Create thread struct and initialize fields
	struct pet_thread * new_thread = (struct pet_thread*)malloc(sizeof(struct pet_thread));
	new_thread->id = (pet_thread_id_t)thread_id_count++;
	*thread_id = new_thread->id;
	new_thread->func = func;
	new_thread->state = PET_THREAD_READY;
	new_thread->return_reciever = (uint64_t)NULL;
	list_head_init(&new_thread->waiting_for_me_list);
	assert(new_thread->waiting_for_me_list.next == new_thread->waiting_for_me_list.prev && new_thread->waiting_for_me_list.next == &new_thread->waiting_for_me_list);

	// Allocate a stack
	new_thread->stack_bottom = calloc(STACK_SIZE, 1);
	int num_entries = STACK_SIZE/sizeof(uintptr_t);

	/* Give stack an initial saved context with the following: (The first stack element is
	 * given the __quarantine() address so that, if a thread ever calls "ret" x86 instruction, 
	 * this value will be popped into %rip)
	 *	First stack element <-- __quarantine() address (where threads 'return')
	 *	%rip restore value  <-- thread function address
	 *	%rdi restore value  <-- thread function argument
 	 */
	struct exec_ctx * init_ctx = (struct exec_ctx*)&((uintptr_t *)new_thread->stack_bottom)[num_entries-16];
	init_ctx->rip = (uintptr_t)func;
	init_ctx->rbp = (uintptr_t)&((uintptr_t *)new_thread->stack_bottom)[num_entries-1];
	init_ctx->rdi = (uintptr_t)arg;
	((uintptr_t *)new_thread->stack_bottom)[num_entries-1] = (uintptr_t)__quarantine;

	// Point stack_rsp to end of saved context
	new_thread->stack_rsp = (void *)&((uintptr_t *)new_thread->stack_bottom)[num_entries-16];

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
	DEBUG("Entered pet_thread_cleanup(): prev_id=%d, my_id=%d\n", (int)prev_id, (int)my_id);
	__dump_stack(get_thread(my_id));
	__dump_stack(get_thread(prev_id));
	struct pet_thread * prev_thread = get_thread(prev_id);
	if(prev_thread->state == PET_THREAD_STOPPED) {
		// De-allocate thread memory
		DEBUG("CLEANING THREAD %d\n", (int)prev_thread->id);
		free(prev_thread->stack_bottom);
		free(prev_thread);
	}
}


/* 
 * Puts running thread in running queue, 
 */
static void
__yield_to(struct pet_thread * tgt_thread)
{
	if(tgt_thread==NULL || tgt_thread->state != PET_THREAD_READY) {
		DEBUG("WARNING: Thread yielded to is not ready, or doesn't exist (may have exited). Scheduling another\n");
		pet_thread_schedule();
		return; // returning point A... Continue with func
	}
	DEBUG("FROM __yield_to: calling __thread_invoker(id=%d)\n", (int)tgt_thread->id);
	__thread_invoker(tgt_thread);
	// returning point B... Continue with func
}


int
pet_thread_yield_to(pet_thread_id_t thread_id)
{
	DEBUG("ENTERED pet_thread_yield_to (YT thread_id=%d, running->id=%d)\n", (int)thread_id, (int)running->id);
	__yield_to(get_thread(thread_id));

	return 0;
}



int
pet_thread_schedule()
{
	// If ready queue is empty, schedule master_dummy_thread
	if(list_empty(&ready_list) && running!=&master_dummy_thread) {
		__thread_invoker(&master_dummy_thread);
		return 9;	// NEVER REACHED
	}

	while( !list_empty(&ready_list) ) {
		dump_list(&ready_list, "Ready List");

		struct pet_thread *pos;
		list_for_each_entry(pos, &ready_list, list) {
			assert(pos->state == PET_THREAD_READY);
			DEBUG("calling __thread_invoker(id=%d)\n", (int)pos->id);
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
	     
