# User Level Threads

As far as we know, everything required for this project works as expected. 
Running threads with valgrind reveals no memory leaks. We now present an 
overview of some design decisions we made for this project. 

## Scheduling threads
We employ a basic round robin. When a thread is made READY, it is placed at the
tail of ready\_list. pet\_thread\_schedule then takes an item from the head of
ready_list and calls __thread_invoker() on it. 

## Managing blocked threads
asdf as
