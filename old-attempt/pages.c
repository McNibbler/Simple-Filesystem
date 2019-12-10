// Thomas Kaunzinger
// Pages implementation

//Imports
#include "pages.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "bitmap.h"
#include <string.h>
#include <assert.h>

// Global variables
void* head;
int fd = -1;
void* bitmap;
int initialized;	// Boolean to make sure init was called

// TODO: rv checks, size checks

// Call at the start of the program
void pages_init(const char* path) {
	fd = open(path, O_CREAT | O_RDWR, PERMISSIONS);
	assert(fd >= 0);
	int ret = ftruncate(fd, TOTAL_SIZE);
	assert(!ret);
	// Taken from example from hw06
	head = mmap(0, TOTAL_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	assert(head != MAP_FAILED);
	// Initializes root page
	bitmap = get_pages_bitmap();
	bitmap_put(bitmap, 0, 1);
	initialized = 1;
}

// Frees the whole filesystem
void pages_free() {
	if (initialized) {
		int ret = munmap(head, TOTAL_SIZE);
		assert(!ret);
	}
}

// Gets the page at the specified index
void* pages_get_page(int pnum) {
	/*
	if (!initialized) {
		return 0;
	}*/
	return (void*)(((char*)head) + (pnum * BLOCK_SIZE));
}

// The blocks bitmap is at the start before the inode bitmap
void* get_pages_bitmap() {
	/*if (!initialized) {
		return 0;
	}*/
	return pages_get_page(0);
}

// the inode bitmap is immediately after the blocks bitmap
void* get_inode_bitmap() {
	/*if (!initialized) {
		return 0;
	}*/
	return (void*)(((char*)pages_get_page(0)) + (NUM_BLOCKS / 8));
}

// Allocates the available pages
int alloc_page() {
	/*
	if (!initialized) {
		return 0;
	}*/

	for (int ii = 0; ii < NUM_BLOCKS; ii++) {
		if (!bitmap_get(bitmap, ii)) {
			// Clears the space of its data and then flags the block as used
			memset(pages_get_page(ii), 0, BLOCK_SIZE);
			bitmap_put(bitmap, ii, 1);
			return ii;
		}
	}
	return -ENOSPC;
}

// Clears the space in the page and subsequently flags the block as used
void free_page(int pnum) {
	if (initialized) {
		memset(pages_get_page(pnum), 0, BLOCK_SIZE);
		bitmap_put(bitmap, pnum, 0);
	}
}

