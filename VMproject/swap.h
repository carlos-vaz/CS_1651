/* 
 * Copyright (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 * All rights reserved.
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "PETLAB_LICENSE".
 */

#ifndef __PETMEM_SWAP_H__
#define __PETMEM_SWAP_H__

/* Swap file implementation */


#include <linux/fs.h>

struct swap_space {
    struct file * swap_file;

    u32 num_pages;
};


struct swap_space * swap_init(u32 num_pages);
void swap_free(struct swap_space * swap);


int swap_out_page(struct swap_space * swap, u32 * index, void * page);
int swap_in_page (struct swap_space * swap, u32   index, void * dst_page);


#endif
