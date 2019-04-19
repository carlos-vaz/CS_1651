# User Level Threads

As far as we know, everything required for this project works as expected. 
Running threads with valgrind reveals no memory leaks. We now present an 
overview of some design decisions we made for this project. 

## High level thread management
We employ a simple round robin to schedule threads. When a thread becomes ready (after creation, for example), it is placed at the
tail of ready_list. pet_thread_schedule() then takes an item from the head of ready_list and calls __thread_invoker() on it. 
__thread_invoker() is in charge of moving threads from state to state. It manages three different storage locations: ``ready_list",
the blocked_list, and the running pointer. __thread_invoker() 


