// Thomas Kaunzinger
// iNode implementation

// Imports
#include "inode.h"
#include "pages.h"
#include <errno.h>
#include <string.h>
#include "bitmap.h"
#include "util.h"
#include <assert.h>

// Prints the contents of an iNode
void print_inode(inode* node) {
	printf("Num Refs: %d", node->refs);
	printf("Mode    : %o", node->mode);
	printf("Size    : %d", node->size);
	printf("Pointers: %x, %x", node->ptrs[0], node->ptrs[1]);
	printf("Indirect: %d", node->iptr);
}

// To allocate space for the inodes
void inode_initialize() {
	int isRootInitialized = bitmap_get(get_inode_bitmap(), 0);
	if (!isRootInitialized) {
		for (int ii = 1; ii <= bytes_to_pages(sizeof(inode) * NUM_BLOCKS); ii++) {
			int rv = alloc_page();
			assert(rv == ii);
		}	
	}
}

// Returns an inode from the dedicated inode table after the bitmaps
inode* get_inode(int inum) {
	// iNodes are stored in the page following the bitmaps in the mmap
	if (inum < 0 || inum >= NUM_BLOCKS || !bitmap_get(get_inode_bitmap(), inum)) {
		return 0;
	}
	return ((inode*)pages_get_page(1) + inum);
}

// Allocates an inode be initialized and returns which index of the node is free
int alloc_inode() {
	void* bitmap = get_inode_bitmap();
	// Pretty much identical to how I did it in pages.c
	for (int ii = 0; ii < NUM_BLOCKS; ii++) {
		if (!bitmap_get(bitmap, ii)) {
			bitmap_put(bitmap, ii, 1);
			return ii;
		}
	}
	return -ENOSPC;
}

// TODO: I'm sure I'll need to do more here for the challenge 
void free_inode(int inum) {
	void* bitmap = get_inode_bitmap();
	inode* in = get_inode(inum);

	// inum 0 is root so no dont delete that pls. Also prevents double free.
	if (!inum || !in || !bitmap_get(bitmap, inum)) {
		return;
	}

	in->mode = 0;
	in->size = 0;
	for (int ii = 0; ii < DIR_PTRS; ii++) {
		free_page(in->ptrs[ii]);
	}
	memset(in->ptrs, 0, sizeof(in->ptrs));
	if (in->iptr) {
		free_inode(in->iptr);
	}
	in->iptr = 0;
	bitmap_put(bitmap, inum, 0);
}

// Grows an inod4 to allocate more space in its direct and indirect pointers
int grow_inode(inode* node, int size) {

	if (node->size < size) {
		return -1;
	}

	if (bytes_to_pages(size) <= DIR_PTRS) {
		for (int ii = 0; ii < bytes_to_pages(size); ii++) {
			int newPage = alloc_page();
			if (newPage == -ENOSPC) {
				return -ENOSPC;
			}
			node->ptrs[ii] = newPage;
		}
		return 0;
	}
	// TODO: Check?
	else {
		if (!node->iptr) {
			int indirect = alloc_inode();
			if (indirect == -ENOSPC) {
				return -ENOSPC;
			}
			inode* indir_in = get_inode(indirect);
			node->iptr = indirect;
			return grow_inode(indir_in, size - (DIR_PTRS * BLOCK_SIZE));
		}
		// Is this correct?
		else {
			return grow_inode(get_inode(node->iptr), size - (DIR_PTRS * BLOCK_SIZE));
		}
	}
}

// TODO: I don't need to do this yet
int shrink_inode(inode* inode, int size) {
	return -1;
}

// TODO: I'm not actually sure this is correct
int inode_get_pnum(inode* node, int fpn) {
	return 1 + ((NUM_BLOCKS * sizeof(inode)) / BLOCK_SIZE) + fpn;
}
