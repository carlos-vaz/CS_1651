Adam Grusky
Carlos Vazquez Gomez


The following functionalities passed all of our tests: 
	-> Initialization of memory map (petmem_init_process)
	-> Allocation of virtual memory (petmem_alloc_vspace)
      	-> Freeing of virtual & physical memory (petmem_free_vspace)
	-> Assignment of physical pages (petmem_handle_pagefault)
	-> Accessing SIGSEV sifinfo to determine if user breached permission 

We ran into the following issues with the following functionalities: 
	->  When freeing pages that were used for tables (petmem_free_vspace), 
	    occasionally a table that is supposed exclusively cover vspace 
	    from the allocation we are freeing still contains an entry marked
	    'present' after reaching the end of the table. This prevents us 
	    from freeing the table page. 

Our main challenge was in de-allocating pages used as page tables. We implemented a depth-first 
search of the page table tree, keeping track of the number of page-sized chunks of virtual memory 
we have covered so far. When the search reaches a page table entry that is the last entry of 
that table, we free the page table page (if it belongs entirely to us). We also free user-space 
pages if the search reaches any PTEs whose present bit is set. 


We successfully ran our code on Ubuntu 18 Desktop, Ubuntu 16 Server, 
and Ubuntu 18 Server. 