# User Level Threads
As far as we know, everything required for this project works as expected. 
Running threads with valgrind reveals no memory leaks. We now present an 
overview of some design decisions we made for this project. 

## High level thread management
We employ a simple round robin to schedule threads. When a thread becomes ready (after creation, for example), it is placed at the
tail of ready_list. pet_thread_schedule() then takes an item from the head of ready_list and calls __thread_invoker() on it. 
__thread_invoker() is in charge of moving threads from state to state. It manages three different storage locations: "ready_list",
"blocked_list", and the "running" pointer. __thread_invoker() takes the currently running thread, reads its state to determine which list
to add it to, and puts the target thread in "running" and changes its state to PET_THREAD_RUNNING. 

## Joining threads
Each thread struct has a list_head (waiting_for_me_list) dedicated to keeping a list of threads that are blocked waiting for it to exit. 
When a thread calls pet_thread_join(), it adds itself to the waiting_for_me_list of the joined-to thread. When the joined-to thread 
eventually exits, it invariably checks its waiting_for_me_list to unblock (and place in ready_list) all threads in it. 

## Initial thread context
We write to the top of each fresh stack an initial context with 16 64-bit values. When this fresh stack is invoked for the first time via 
__switch_to_stack(), this function will pop 14 of these values to their appropriate registers. We seed the stack so that the argument to the 
thread function so that it gets popped to %rdi, and the pointer to the top of the stack gets popped to $rbp. We also place the pointer to the 
thread function such that when __switch_to_stack() returns, this value gets popped into %rip, thus jumping to the thread function code. Lastly, we
seed the stack with a pointer to a custom made assembly function: __capture_return() (explained below)

## Returning values from joined-to threads
We implemented a nifty scheme to capture return values. A thread has two ways of exiting (both can return a value): 
	1. Placing return value into %rax and calling the "retq" x86 instruction. This jumps to __capture_return() (as explained above). By calling "return" in the thread function, the compiler will first copy the return value to %rax (if a 64 bit value), 
then it will change stack pointer to point to the value of %rbp, which will put the rsp right above the stack entry containing __capture_return. 
Lastly, the compiler inserts a "retq" instruction which will pop __capture_return into %rip, thus jumping to it. Inside __capture_pointer, we 
immediately move the contents of %rax to %rdi (the argument to the next function) and call __quarantine(), which will eventually copy the argument 
to the threads that need it. By writing __capture_return in assembly, we prevent the compiler from changing (if it wished) %rax before we could get 
to it, since the compiler has that right (%rax is a caller saver register). 

	2. Calling pet_thread_exit() with the return value as the argument. Inside pet_thread_exit(), __quarantine() is called with the ret_val 
		as the argument
