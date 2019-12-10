// Thomas Kaunzinger
// Directory implementation

// imports
#include <assert.h>
#include "directory.h"
#include "pages.h"
#include "inode.h"
#include <errno.h>
#include "bitmap.h"
#include <string.h>
#include "util.h"
#include <sys/stat.h>

const int ROOT_PERMISSIONS =  __S_IFDIR | 0755;	// All can read and execute, but only root can write

// Initializes the root directory
void directory_init() {
	// The root sure as hell better not be set already
	int rv = bitmap_get(get_inode_bitmap(), 0);
	if(rv) {
		return;	
	}
	int root_inode_index = alloc_inode();
	assert(!root_inode_index);
	inode* root_inode = get_inode(root_inode_index);
	
	// Initializes inode
	root_inode->refs = 0;
	root_inode->mode = ROOT_PERMISSIONS;
	root_inode->size = 0;	// TODO: more for ch3?
	memset(root_inode->ptrs, 0, sizeof(root_inode->ptrs));	// TODO: This is likely not correct
	root_inode->iptr = 0;

	// Redirects back to root
	directory_put(root_inode, "..", 0);
	directory_put(root_inode, ".", 0);
}

// Finds the directory entry of a path in a given directory
dirent* directory_lookup_dirent(inode* dd, const char* name) {

	if (streq(name, "/")) {
		return (directory_lookup_dirent(dd, "."));
	}
	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = dd->ptrs[ii];
		if (ptr > 0) {
			dirent* pg = pages_get_page(ptr);
			for (int jj = 0; jj < BLOCK_SIZE / sizeof(dirent); jj++) {
				if (streq(pg[jj].name, name)) {
					return &(pg[jj]);
				}
			}
		}
	}
	// Null if none found
	if (dd->iptr < 1) {
		return 0;	// TODO: correct?
	}
	else {
		return directory_lookup_dirent(get_inode(dd->iptr), name);
	}
}

// Finds the inode index of a path in a given directory
int directory_lookup(inode* dd, const char* name) {
	dirent* de = directory_lookup_dirent(dd, name);
	if (!de) {
		return -ENOENT;
	}
	return de->inum;
}

// Finds the inode index of a path
// TODO: not just root
int tree_lookup(const char* path) {
	return directory_lookup(get_inode(0), path);
}

// Puts an inode into the directory
int directory_put(inode* dd, const char* name, int inum) {
	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = dd->ptrs[ii];
		if (!ptr) {
			ptr = alloc_page(); 
			if (ptr < 0) {
				return -ENOSPC;
			}
			dd->size += BLOCK_SIZE;
			dd->ptrs[ii] = ptr;
		}
		for (int jj = 0; jj < BLOCK_SIZE / sizeof(dirent); jj++) {
			dirent* pg = pages_get_page(ptr);
			if (streq(pg[jj].name, "")) {	// TODO: is this the same as NULL?
				strncpy((pg + jj)->name, name, DIR_NAME);
				(pg + jj)->inum = inum;
				return 0;
			}
		}
	}
	if (!dd->iptr) {
		int status = grow_inode(dd, dd->size + BLOCK_SIZE);
		if (status < 0) {
			return -ENOSPC;
		}
	}
	return directory_put(get_inode(dd->iptr), name, inum);
}

// Deletes an item in the desired directory
int directory_delete(inode* dd, const char* name) {
	/*
	int delindex = directory_lookup(get_inode(0), name);
	if (delindex < 0) {
		return -ENOENT;
	}
	*/

	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = dd->ptrs[ii];
		if (ptr > 0) {
			for (int jj = 0; jj < BLOCK_SIZE / sizeof(dirent); jj++) {
				dirent* pg = pages_get_page(ptr);
				if (streq(pg[jj].name, name)) {
					strncpy((pg + jj)->name, "", DIR_NAME);
					(pg + jj)->inum = 0;
					return 0;
				}
			}
		}
	}
	if (!dd->iptr) {
		return -ENOENT;
	}
	else {
		return directory_delete(get_inode(dd->iptr), name);
	}
}

// Helper function for directory_list
slist* dl_help(inode* dd, slist* sl) {
	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = dd->ptrs[ii];
		if (ptr > 0) {
			for (int jj = 0; jj < BLOCK_SIZE / sizeof(dirent); jj++) {
				dirent* pg = pages_get_page(ptr);
				if (!streq(pg[jj].name, "")) {
					sl = s_cons(pg[jj].name, sl);
				}
			}
		}
	}
	if (dd->iptr) {
		return dl_help(get_inode(dd->iptr), sl);
	}
	return sl;
}
// Creates a list of the items in the directory
slist* directory_list(const char* path) {
	inode* dd = get_inode(0);
	return dl_help(dd, 0);
}

// Prints the items in a directory for debugging
void print_directory(inode* dd) {
	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = dd->ptrs[ii];
		if (ptr) {
			for (int jj = 0; jj < BLOCK_SIZE / sizeof(dirent); jj++) {
				dirent* pg = pages_get_page(ptr);
				if (!streq(pg[jj].name, "")) {
					printf("%s\n", (pg + jj)->name);
				}
			}
		}
	}
	if (dd->iptr) {
		print_directory(get_inode(dd->iptr));
	}
}



