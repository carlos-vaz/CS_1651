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
 * Sets 'expected' to '*ptr' if '*ptr' != 'expected' (needed for determining who locked)
 */
unsigned int
compare_and_swap(unsigned int * ptr,
		 unsigned int * expected,
		 unsigned int   new)
{
	unsigned int original = *ptr;
	asm ("lock cmpxchg %2, %0;\n" : "+m" (*ptr), "+a" (*expected) : "r" (new));
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
	unsigned int expect = 0;
	while( expect == 0 ) {
		expect = 1;
		compare_and_swap(&(lock->free), &expect, 0);
	}
	// If got here, expect == 1 & lock->free = 0 (your lock);
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
    /* Implement this */
    return 0;
}

/* Exercise 4:
 *     Barrier operations
 */

void
barrier_init(struct barrier * bar,
	     int              count)
{
	bar->init_count = count;
}

void
barrier_wait(struct barrier * bar)
{
	
}


/* Exercise 5:
 *     Reader Writer Locks
 */

void
rw_lock_init(struct read_write_lock * lock)
{
	lock->num_readers = 0;
	lock->writer = 0;
	lock->mutex = malloc(sizeof(struct spinlock));
}


void
rw_read_lock(struct read_write_lock * lock)
{
    /* Implement this */
}

void
rw_read_unlock(struct read_write_lock * lock)
{
    /* Implement this */
}

void
rw_write_lock(struct read_write_lock * lock)
{
    /* Implement this */
}


void
rw_write_unlock(struct read_write_lock * lock)
{
    /* Implement this */
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
    /* Implement this */
}



void
lf_queue_init(struct lf_queue * queue)
{
    /* Implement this */
}

void
lf_queue_deinit(struct lf_queue * lf)
{
    /* Implement this */
}

void
lf_enqueue(struct lf_queue * queue,
	   int               val)
{
    /* Implement this */
}

int
lf_dequeue(struct lf_queue * queue,
	   int             * val)
{
    /* Implement this */
    return 0;
}



