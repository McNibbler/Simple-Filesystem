// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>

// Constants
#define BLOCK_SIZE 0x1000	// 1 block = 1 page
#define NUM_BLOCKS 0x100	// 256 blocks * 4096 bytes per block = 1MB 
#define TOTAL_SIZE 0x100000
#define PERMISSIONS 0644	// R/W owner, R group, R all

// Initializes the pages for the memory maps
void pages_init(const char* path);
void pages_free();
void* pages_get_page(int pnum);
void* get_pages_bitmap();
void* get_inode_bitmap();
int alloc_page();
void free_page(int pnum);

#endif
