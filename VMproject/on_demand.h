/* 
 * Copyright (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 * All rights reserved.
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "PETLAB_LICENSE".
 */
 
/* On demand Paging Implementation */

#include <linux/module.h>

#include "swap.h"

struct mem_map {
	struct list_head list;
	unsigned long start;
	unsigned long size;
	int allocated;
	int head;
};

struct mem_map * petmem_init_process(void);
void petmem_deinit_process(struct mem_map * map);

uintptr_t petmem_alloc_vspace(struct mem_map * map, u64 num_pages);
void petmem_free_vspace(struct mem_map * map, uintptr_t vaddr);

void petmem_free_vspace(struct mem_map *, uintptr_t);
pte64_t * walk_page_table(uintptr_t);

void petmem_dump_vspace(struct mem_map * map);

// How do we get error codes??
int petmem_handle_pagefault(struct mem_map * map, uintptr_t fault_addr, u32 error_code);
