#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "locking.h"




/* Exercise 1:
 *     Basic memory barrier
 */
void mem_barrier() {
    /* Implement this */
}


/* Exercise 2: 
 *     Simple atomic operations 
 */

void
atomic_sub( int * value,
	    int   dec_val)
{
    /* Implement this */
}

void
atomic_add( int * value,
	    int   inc_val)
{
    /* Implement this */
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
    /* Implement this */

    return 0;
}

void
spinlock_init(struct spinlock * lock)
{
    /* Implement this */
}

void
spinlock_lock(struct spinlock * lock)
{
    /* Implement this */
}


void
spinlock_unlock(struct spinlock * lock)
{
    /* Implement this */
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
    /* Implement this */
}

void
barrier_wait(struct barrier * bar)
{
    /* Implement this */
}


/* Exercise 5:
 *     Reader Writer Locks
 */

void
rw_lock_init(struct read_write_lock * lock)
{
    /* Implement this */
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



