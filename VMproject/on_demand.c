/* On demand Paging Implementation
 * (c) Jack Lange, 2012
 */

#include <linux/slab.h>


#include "petmem.h"
#include "on_demand.h"
#include "pgtables.h"
#include "swap.h"


struct mem_map *
petmem_init_process(void)
{
	// Create entry mem_map for LIST_HEAD
	struct mem_map * map0 = kmalloc(sizeof(struct mem_map), GFP_KERNEL);
	INIT_LIST_HEAD(&map0->list);
	map0->head = 1;

	// Create contents mem_map containing all free memory
	struct mem_map * map = kmalloc(sizeof(struct mem_map), GFP_KERNEL);
	map->allocated = 0;
	map->start = PETMEM_REGION_START;
	map->size = PETMEM_REGION_END - PETMEM_REGION_START;
	map->head = 0;
	list_add(&map->list, &map0->list);
	
	return map0;
}


void
petmem_deinit_process(struct mem_map * map)
{
	// Iterate through mem_map, free memory if allocated, destroy mem_map nodes
	// Called when user calls close(fd)?
	struct mem_map *cursor;
	list_for_each_entry(cursor, &map->list, list) {
		
	}
}


struct mem_map * petmem_find_address(struct mem_map * map, unsigned long vaddr) {
	struct mem_map *cursor;
	list_for_each_entry(cursor, &map->list, list) {
		if(cursor->allocated==1 && vaddr>=cursor->start && vaddr<cursor->start+cursor->size) 
			return cursor;
	}
	return NULL;
}

uintptr_t
petmem_alloc_vspace(struct mem_map * map,
		    u64              num_pages)
{
	struct mem_map *cursor;
	unsigned long last_start = PETMEM_REGION_START;
	unsigned long last_size = PETMEM_REGION_END - PETMEM_REGION_START;
	printk("Memory allocation\n");
	list_for_each_entry(cursor, &map->list, list) {
		printk("Inside list_for_each: alloc=%d, size=%lu\n", cursor->allocated, cursor->size);
		if(cursor->allocated == 0 && cursor->size/PAGE_SIZE_4KB >= num_pages) {
			unsigned long combined_size = cursor->size;
			cursor->size = PAGE_SIZE_4KB*num_pages;
			cursor->allocated = 1;
			struct mem_map * new_map = kmalloc(sizeof(struct mem_map), GFP_KERNEL);
			new_map->start = cursor->start + cursor->size;
			new_map->size = combined_size - cursor->size;
			new_map->allocated = 0;
			new_map->head = 0;
			list_add(&new_map->list, &cursor->list);
			return (uintptr_t)cursor->start;
		}
	}
	printk("petmem_alloc_vspace: Memory allocation impossible (no adequate free memory region found)\n");
	return NULL;
}

void
petmem_dump_vspace(struct mem_map * map)
{
	printk("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");
	printk("-- -- -- -- --  MEMORY MAP DUMP  -- -- -- -- --\n");
	printk("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n\n");

	struct mem_map *cursor;
	int i=0;
	list_for_each_entry(cursor, &map->list, list) {
		printk("%sNode %d:\tAllocated = %d\n\tStart = %lx\n\tSize = %lu\n\n", 
		cursor->head==1 ? "[HEAD] " : "" , i, cursor->allocated, cursor->start, cursor->size);
		i++;
	}

    return;
}




// Only the PML needs to stay, everything else can be freed
void
petmem_free_vspace(struct mem_map * map,
		   uintptr_t        vaddr)
{
	struct mem_map *cursor, *cursor_next, *cursor_prev;
	unsigned long free_start, free_size;
	int i=0, found=0;
	printk("Freeing Memory\n");
	// Free the virtual memory
	list_for_each_entry(cursor, &map->list, list) {
		if(cursor->start==vaddr && cursor->allocated==1) {
			printk("Freeing mem_map node %d\n", i);
			found = 1;
			break;
		}
		i++;
	}
	// If not mapped, do not free
	if (found==0) {
		printk("petmem_free_vspace: user tried to free memory that was not mapped!\n");
		return;
	}
	free_start = cursor->start;
	free_size = cursor->size;
	printk("Indeed: freeing mem_map node %d\n", i);
	cursor->allocated = 0;
	cursor_prev = list_entry(cursor->list.prev, struct mem_map, list);
	cursor_next = list_entry(cursor->list.next, struct mem_map, list);
	printk("(cursor_prev start = %lx)\n", cursor_prev->start);
	printk("(cursor_next start = %lx)\n", cursor_next->start);
	if(cursor_prev->allocated==0 && cursor_prev->head==0) {
		// Combine with cursor_prev
		printk("Combine with cursor_prev\n");
		cursor_prev->size += cursor->size;
		list_del(&cursor->list);
		cursor = cursor_prev;
	}
	if(cursor_next->allocated==0 && cursor_next!=cursor_prev) {
		// combine with cursor_next
		printk("Combine with cursor_next (cursor_next start = %lx)\n", cursor_next->start);
		cursor_next->start = cursor->start;
		cursor_next->size += cursor->size;
		list_del(&cursor->list);
	}
	

	// Free the physical pages
	// TODO: Free the page table pages too
	int freed = 0, num_pages = free_size/PAGE_SIZE_4KB;
	pte64_t * pte;
	unsigned long user_page;
	for(i=0; i<num_pages; i++) {
		pte = (	pte64_t * )walk_page_table((uintptr_t)free_start+i*PAGE_SIZE_4KB);
		if(pte->present == 0)
			continue;
		freed++;
		pte->present = 0;
		user_page = __va(BASE_TO_PAGE_ADDR(pte->page_base_addr));
		invlpg(user_page);
		// TODO: change to buddy system
		free_page(user_page);
	}
	printk("Freed %d pages from mem_map node of size %d pages\n", freed, num_pages);
	return;
}


void * walk_page_table(uintptr_t fault_addr) {


	// Grab cr3
	unsigned long cr3 = get_cr3();
	printk("CR3 = %lx\n", cr3);
	//cr3 = CR3_TO_PML4E64_PA(cr3);
	//printk("CR3_TO_PML4E64_PA(CR3) = %lx\n", cr3);


	// Different experiments for values of fault_addr
	//fault_addr = CR3_TO_PML4E64_VA(cr3);
	//fault_addr = 0xffff93efffffffff;
	/*fault_addrr = kmalloc(12, GFP_KERNEL);
	char * fault_addr = (char *)fault_addrr;
	printk("---- WRITING DATA TO PAGE AT FAULT_ADDR ");
	fault_addr[0] = 'y';
	printk("(read back '%c')  ----\n", fault_addr[0]);*/
	
	// VA --> PML4E64 Index
	unsigned long pml_index = PML4E64_INDEX(fault_addr);
	printk("PML4E64 Index = %lu\n", pml_index);

	// VA --> PDPE64 Index
	unsigned long pdp_index = PDPE64_INDEX(fault_addr);
	printk("PDPE64 Index = %lu\n", pdp_index);

	// VA --> PDE64 Index
	unsigned long pde_index = PDE64_INDEX(fault_addr);
	printk("PDE64 Index =  %lu\n", pde_index);

	// VA --> PTE64 Index
	unsigned long pte_index = PTE64_INDEX(fault_addr);
	printk("PTE64 Index =  %lu\n", pte_index);


	printk("------- Corrections ------\n");
	void * va_test = NULL;
	printk("__va Test: phys: %lx, \tvirt: %lx\n", va_test, __va(va_test));
	printk("------- End of __va test ------\n");
	pml4e64_t * pml_dest;
	pdpe64_t  * pdp_dest;
	pde64_t   * pde_dest;
	pte64_t   * pte_dest;
	unsigned long pdp_table_pg;
	unsigned long pde_table_pg;
	unsigned long pte_table_pg;

	
	pml_dest = CR3_TO_PML4E64_VA(cr3) + pml_index*sizeof(pml4e64_t);
	//pml_dest = __va(cr3) + pml_index*sizeof(pml4e64_t);	
	printk("pml_dest = %lx\n", pml_dest);
	printk("pml_dest->present = %d\n", pml_dest->present);
   printk("(1.) pml_dest->accessed (pdp table accessed) = %d\n", pml_dest->accessed);
   pml_dest->accessed = 0;
   //invlpg(CR3_TO_PML4E64_PA(cr3)); // invl pml page 
	if(pml_dest->present == 0) {
		printk("PDP TABLE PAGE NOT PRESENT... WRITING\n");
		// Allocate page for PDP table
		pdp_table_pg = get_zeroed_page(GFP_KERNEL);
		printk("Received page for pdp table @ %lx\n", pdp_table_pg);
		// Create PML entry
		pml_dest->present = 1;
		pml_dest->writable = 1;
		pml_dest->user_page = 1;
		pml_dest->pdp_base_addr = PAGE_TO_BASE_ADDR(__pa(pdp_table_pg));
		printk("PML Entry: present = %d\n", pml_dest->present);
		printk("PML Entry: pdp_base_addr = %lx\n", pml_dest->pdp_base_addr);
	}
	pdp_dest = __va(BASE_TO_PAGE_ADDR(pml_dest->pdp_base_addr)) + pdp_index*sizeof(pdpe64_t);
	printk("pdp_dest = %lx\n", pdp_dest);
	printk("pdp_dest->present = %d\n", pdp_dest->present);
	if(pdp_dest->present == 0) {		
		printk("PDE TABLE PAGE NOT PRESENT... WRITING\n");
		// Allocate page for PDE table
		pde_table_pg = get_zeroed_page(GFP_KERNEL);
		printk("Received page for pde table @ %lx\n", pde_table_pg);	
		// Create PDP entry
		pdp_dest->present = 1;
		pdp_dest->writable = 1;
		pdp_dest->user_page = 1;
		pdp_dest->pd_base_addr = PAGE_TO_BASE_ADDR(__pa(pde_table_pg));
		printk("PDP Entry: present = %d\n", pdp_dest->present);
		printk("PDP Entry: pd_base_addr = %lx\n", pdp_dest->pd_base_addr);
	}
   //invlpg(CR3_TO_PML4E64_PA(cr3)); // invl pml page 
   printk("(2.) pml_dest->accessed (pdp table accessed) = %d\n", pml_dest->accessed);
   pml_dest->accessed = 0;
	pde_dest = __va(BASE_TO_PAGE_ADDR(pdp_dest->pd_base_addr)) + pde_index*sizeof(pde64_t);
	printk("pde_dest = %lx\n", pde_dest);
	printk("pde_dest->present = %d\n", pde_dest->present);
	if(pde_dest->present == 0) {
		printk("PTE TABLE PAGE NOT PRESENT... WRITING\n");
		// Allocate page for PTE table
		pte_table_pg = get_zeroed_page(GFP_KERNEL);
		printk("Received page for pte table @ %lx\n", pte_table_pg);	
		// Create PDE entry
		pde_dest->present = 1;
		pde_dest->writable = 1;
		pde_dest->user_page = 1;
		pde_dest->pt_base_addr = PAGE_TO_BASE_ADDR(__pa(pte_table_pg));
		printk("PDE Entry: present = %d\n", pde_dest->present);
		printk("PDE Entry: pt_base_addr = %lx\n", pde_dest->pt_base_addr);
	}
	pte_dest = __va(BASE_TO_PAGE_ADDR(pde_dest->pt_base_addr)) + pte_index*sizeof(pte64_t);
	return pte_dest;

}


/* 
   error_code (derived from siginfo_t struct filled in by kernel upon segfault):
       1 == not present
       2 == permissions error
*/

int
petmem_handle_pagefault(struct mem_map * map,
			uintptr_t        fault_addr,
			u32              error_code)
{
	if(error_code==2) {
		printk("petmem_handle_pagefault: Intercepted segfault, but it was due to a permission error.\nPlease prepare to die.\n");
		return -1;
	}
	printk("petmem_handle_pagefault: Page fault! At address\t %lx\n", fault_addr);
	printk("petmem_handle_pagefault: Is this address allocated in our memory map?\n");
	if(petmem_find_address(map, (unsigned long)fault_addr) == NULL) {
		printk("petmem_handle_pagefault: Address %lx not mapped (returning 1)\n", fault_addr);
		printk("petmem_handle_pagefault: Please prepare to die\n");
		return -1;
	}

	unsigned long zeroed_user_pg;
	pte64_t * pte_dest = (pte64_t *)walk_page_table(fault_addr);

	printk("pte_dest = %lx\n", pte_dest);
	printk("pte_dest->present = %d\n", pte_dest->present);
	if(pte_dest->present == 0) {
		printk("USER ACCESSED PAGE NOT PRESENT... WRITING\n");
		// Allocate ZEROED! page for user
		zeroed_user_pg = get_zeroed_page(GFP_KERNEL);
		//zeroed_user_pg = (unsigned long)petmem_alloc_pages(1);
		printk("Received BUDDY page for user @ %lx\n", zeroed_user_pg);
		// Create PTE entry
		pte_dest->present = 1;
		pte_dest->writable = 1;
		pte_dest->user_page = 1;
		pte_dest->page_base_addr = PAGE_TO_BASE_ADDR(__pa(zeroed_user_pg));

		printk("PTE Entry: present = %d\n", pte_dest->present);
		printk("PTE Entry: page_base_addr = %lx\n", pte_dest->page_base_addr);

		// Invalidate PTE entry in case TLB cached it
		invlpg(zeroed_user_pg); // TODO: Why would it cache?
		
	}
	else {
		// page faulted, but present. Should never be here. 
		printk("YOU WILL NEVER SEE THIS: page faulted, but present\n");
		// TODO: Is this the case where user has no permission?
	}
	flush_tlb(); // TODO: Remove this

	return 0;
}


