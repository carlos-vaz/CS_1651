#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "locking.h"




/* Exercise 1:
 *     Basic memory barrier
 */
void mem_barrier(void *p) {
	asm (""::"m" (*p));
}


/* Exercise 2: 
 *     Simple atomic operations 
 */

void
atomic_sub( int * value,
	    int   dec_val)
{
	asm ("lock sub %1, %0;\n" : "+m" (*value) : "r" (dec_val));
}

void
atomic_add( int * value,
	    int   inc_val)
{
	asm ("lock add %1, %0;\n" : "+m" (*value) : "r" (inc_val));
}



/* Exercise 3:
 *     Spin lock implementation
 */


/* Compare and Swap
 * Returns the value that was stored at *ptr when this function was called
 * Sets '*ptr' to 'new' if '*ptr' == 'expected'
 */
unsigned int
compare_and_swap(unsigned int * ptr,
		 unsigned int   expected,
		 unsigned int   new)
{
	unsigned int original;
	asm ("lock cmpxchg %2, %0;\n" : "+m" (*ptr), "=a" (original) : "r" (new), "1" (expected));
	return original; 
}

void
spinlock_init(struct spinlock * lock)
{
	lock->free = 1;
}

void
spinlock_lock(struct spinlock * lock)
{
	while(compare_and_swap(&(lock->free), 1, 0)==0);	// spin
}


void
spinlock_unlock(struct spinlock * lock)
{
	lock->free = 1;
}


/* return previous value */
int
atomic_add_ret_prev(int * value,
		    int   inc_val)
{
	int ret;
	asm ("lock xadd %1, %0;\n" : "+m" (*value), "=r" (ret) : "1" (inc_val));
	return ret;
}

/* Exercise 4:
 *     Barrier operations
 */

void
barrier_init(struct barrier * bar,
	     int              count)
{
	bar->init_count = count;
	bar->iterations = 0;
	bar->cur_count  = 0;
}

void
barrier_wait(struct barrier * bar)
{
	int my_iter = bar->iterations;				// save old iterations
	int prev = atomic_add_ret_prev(&(bar->cur_count), 1);	// thread-safe cur_count++
	if(prev == bar->init_count-1) {				// last thread cleans up
		bar->cur_count = 0;					// reset cur_count
		bar->iterations++;					// enable next barrier
		return;
	}
	while(bar->iterations != my_iter+1);			// spin otherwise
	
}


/* Exercise 5:
 *     Reader Writer Locks
 */

void
rw_lock_init(struct read_write_lock * lock)
{
	lock->num_readers = 0;
	lock->writer = 0;
	struct spinlock *sl0 = malloc(sizeof(struct spinlock));
	struct spinlock *sl1 = malloc(sizeof(struct spinlock));
	spinlock_init(sl0);	// sl0->free = 1;
	spinlock_init(sl1);	// sl1->free = 1;
	lock->mutex = sl0;
	lock->w_mutex=sl1;
}


void
rw_read_lock(struct read_write_lock * lock)
{
	spinlock_lock(lock->mutex);
	while(lock->writer);
	lock->num_readers++;
	spinlock_unlock(lock->mutex);
}

void
rw_read_unlock(struct read_write_lock * lock)
{
	atomic_sub(&(lock->mutex), 1);
}

void
rw_write_lock(struct read_write_lock * lock)
{
	spinlock_lock(lock->mutex);
	while(lock->writer || lock->num_readers>0);
	lock->writer = 1;
	spinlock_unlock(lock->mutex);
}


void
rw_write_unlock(struct read_write_lock * lock)
{
	atomic_sub(&(lock->writer), 1);
}



/* Exercise 6:
 *      Lock-free queue
 *
 * see: Implementing Lock-Free Queues. John D. Valois.
 *
 * The test function uses multiple enqueue threads and a single dequeue thread.
 *  Would this algorithm work with multiple enqueue and multiple dequeue threads? Why or why not?
 */


/* Compare and Swap 
 * Same as earlier compare and swap, but with pointers 
 * Explain the difference between this and the earlier Compare and Swap function
 */
uintptr_t
compare_and_swap_ptr(uintptr_t * ptr,
		     uintptr_t   expected,
		     uintptr_t   new)
{
	uintptr_t original;
	asm ("lock cmpxchgq %3, %0;\n" : "+m" (*ptr), "=a" (original) : "1" (expected), "r" (new));
	return original;
} 


void
lf_queue_init(struct lf_queue * queue)
{
	queue->head = malloc(sizeof(struct node));
	queue->tail = malloc(sizeof(struct node));
	queue->head->next = NULL;
	queue->tail->next = NULL;
}

void
lf_queue_deinit(struct lf_queue * lf)
{
	// TODO: c has no garbage collector
}

void
lf_enqueue(struct lf_queue * queue,
	   int               val)
{
	struct node *q = malloc(sizeof(struct node)), *p;
	//uintptr_t addrs_of_tail, addrs_of_p, addrs_of_next;
	q->value = val;
	q->next  = NULL;
	int succ = 0;
	while(succ==0) {
		p = queue->tail;
		compare_and_swap_ptr((uintptr_t*)&(p->next), (uintptr_t)NULL, (uintptr_t)q);
		// TODO: q and p may have been freed by dequeueing thread! 
		succ = (q==p->next);
		if(succ==0)			 	// elim this `if`
			compare_and_swap_ptr((uintptr_t*)&(queue->tail), (uintptr_t)p, (uintptr_t)(p->next));
	}
	compare_and_swap_ptr((uintptr_t*)&(queue->tail), (uintptr_t)p, (uintptr_t)q); // not needed it ^ `if` is gone
}

int
lf_dequeue(struct lf_queue * queue,
	   int             * val)
{
	int succ = 0;
	struct node *p;
	uintptr_t oldhead;
	while(succ==0) {	
		p = queue->head;
		if(p->next == NULL) {
			return 0;
		}
		oldhead = compare_and_swap_ptr((uintptr_t*)&(queue->head), (uintptr_t)p, (uintptr_t)(p->next));
		succ = (((struct node*)oldhead)->next == p->next); // both local vars. no danger. 
		*val = p->next->value;
	}
	return 1;
}



