/* 
 * Copyright (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 * All rights reserved.
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "PETLAB_LICENSE".
 */
 
/* Swap file implementation */

#include <linux/slab.h>


#include "file_io.h"
#include "swap.h"






struct swap_space *
swap_init(u32 num_pages)
{
    
#if 0

    /* At minimum you'll want this... */
    struct swap_space * swap = NULL;
    
    
    swap->swap_file = file_open("/tmp/petmem.swap", O_CREAT | O_RDWR);
    
    if (IS_ERR(swap->swap_file)) {
	printk(KERN_ERR "Error opening swap file\n");
	kfree(swap);
	return NULL;
    }
    
    return swap;
#endif
    
    return NULL;
}



void
swap_free(struct swap_space * swap)
{
    return;
}


int
swap_out_page(struct swap_space * swap,
	      u32               * index,
	      void              * page) {
    return -1;
}




int
swap_in_page(struct swap_space * swap,
	     u32                 index,
	     void              * dst_page)
{

    return -1;
}
