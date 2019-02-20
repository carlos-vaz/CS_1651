/* 
 * Copyright (c) 2017, Jack Lange <jacklange@cs.pitt.edu>
 * All rights reserved.
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "PETLAB_LICENSE".
 */

/* File Interface */

int file_mkdir(const char * pathname, unsigned short perms, int recurse);


/* MODE: 
   O_RDWR
   O_RDONLY
   O_WRONLY
   O_CREAT
   
*/
struct file * file_open(const char * path, int mode);
int file_close(struct file * file_ptr);

unsigned long long file_size(struct file * file_ptr);


unsigned long long file_read(struct file * file_ptr, void * buffer, 
			     unsigned long long length, 
			     unsigned long long offset);

unsigned long long file_write(struct file * file_ptr, void * buffer, 
			      unsigned long long length, 
			      unsigned long long offset);
