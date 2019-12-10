// Thomas Kaunzinger
// Storage Implementation

// Imports
#include "storage.h"
#include "directory.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "bitmap.h"
#include "util.h"
#include <unistd.h>
#include <sys/types.h>
#include "inode.h"

/**
 * struct stat {
 * 	dev_t		st_dev;
 * 	ino_t		st_ino;
 * 	mode_t		st_mode;
 * 	nlink_t		st_nlink;
 * 	uid_t		st_uid;
 * 	gid_t		st_gid;		// Ignoring
 *	dev_t		st_rdev;
 *	off_t		st_size;
 *	blksize_t	st_blksize;
 *	blkcnt_t	st_blocks;
 *
 *	struct timespec st_atim;	// Ignoring for now
 *	struct timespec st_mtim;
 *	struct timespec st_ctim;
 * }
 *
 */


// Mounts the file system 
void storage_init(const char* path) {
	pages_init(path);
	inode_initialize();
	directory_init();
}

// Fills out the storage details of the desired filepath
int storage_stat(const char* path, struct stat* st) {
	int index = directory_lookup(get_inode(0), path);	// TODO: not root
	if (index >= 0) {
		inode* in = get_inode(index); 
		st->st_mode = in->mode;
		st->st_uid = getuid();	// Syscall gets user ID
		st->st_size = in->size;
		return 0;
	}
	else {
		return -ENOENT;
	}
}

// Reads the data from the desired path and fills the buffer
// TODO: FOR THIS FIRST IMPLEMENTATION IT'S JUST ONE PAGE
int storage_read(const char* path, char* buf, size_t size, off_t offset) {
	int index = directory_lookup(get_inode(0), path);
	if (index < 0 || size > BLOCK_SIZE) {
		return -ENOENT;
	}
	inode* in = get_inode(directory_lookup(get_inode(0), path));	// TODO: not root 
	memcpy(buf, pages_get_page(in->ptrs[0]), BLOCK_SIZE);
	return size;
	/*
	int running_size = size;
	char* runningbuf = buf;
	while (running_size > 0) {
		for (int ii = 0; ii < DIR_PTRS; ii++) {
			int ptr = in->ptrs[ii];
			// if (!ptr) {
			// 	return -ENOENT;
			// }
			void* copy_page = pages_get_page(ptr);

		}
	}

	for (int ii = 0; ii < DIR_PTRS; ii++) {
		int ptr = in->ptrs[ii];
		if (!ptr) {
			return 0;
		}
		if (size >= BLOCK_SIZE) {
			memcpy(runningbuf, pages_get_page(ptr), BLOCK_SIZE);
			size -= BLOCK_SIZE;
			runningbuf += BLOCK_SIZE;
		}
		else {
			memcpy(runningbuf, pages_get_page(ptr), size);
			return 0;
		}
	}
	if (!in->iptr) {
		return 0;
	}
	else {
		storage_read(path, runningbuf, 0,0);
	}
	return -1;
*/
}

// Writes data from the buffer to the desired pages in storage as determined from the filepath
// TODO: THIS IS JUST FOR SINGLE PAGE IMPLEMENTATIONS SO FAR
int storage_write(const char* path, const char* buf, size_t size, off_t offset) {
	int index = directory_lookup(get_inode(0), path);
	if (index < 0 || size > BLOCK_SIZE) {
		return -ENOENT;
	}
	inode* out = get_inode(directory_lookup(get_inode(0), path));	// TODO: not root 
	assert(size <= BLOCK_SIZE);		// TODO: bigger files later
	memcpy(pages_get_page(out->ptrs[0]), buf, size);
	return size;
}

// TODO: what is this for?
int storage_truncate(const char* path, off_t offset) {
	return -1;
	/*
	int index = directory_lookup(get_inode(0), path);
	if (index < 0) {
		return -ENOENT;
	}
	inode* in = get_inode(index);
	*/
}

// For touch and mkdir
int storage_mknod(const char* path, int mode) {
	/*
	if (streq(path, "")) {
		return -ENOENT;
	}
	
	int index = directory_lookup(get_inode(0), path);
	if (index < 0) {
		return -ENOENT;
	}
	*/
	int newInodeIndex = alloc_inode();
	int newPageIndex = alloc_page();
	if (newInodeIndex < 0) {
		if (newPageIndex >= 0) {
			free_page(newPageIndex);
		}
		return -ENOSPC;
	}
	if (newPageIndex < 0) {
		if (newInodeIndex >= 0) {
			free_inode(newInodeIndex);
		}
		return -ENOSPC;
	}
	inode* newNode = get_inode(newInodeIndex);
	newNode->mode = mode;
	newNode->ptrs[0] = newPageIndex;

	int ret = directory_put(get_inode(0), path, newInodeIndex);
	assert(!ret);
	return ret;
}

// TODO: do i do this??
int storage_unlink(const char* path) {
	inode* in = get_inode(directory_lookup(get_inode(0), path));	// TODO: not root 
	return -1;
}

// TODO: do i do this??
int storage_link(const char* from, const char* to) {
	inode* in_from = get_inode(directory_lookup(get_inode(0), from));	// TODO: not root 
	return -1;
}

// Renames a file in a directory and returns the status
int storage_rename(const char* from, const char* to) {
	int from_index = directory_lookup(get_inode(0), from);
	if (from_index < 1) {
		return -ENOENT;
	}
	dirent* de = directory_lookup_dirent(get_inode(0), from);
	if (de <= 0) {
		return -ENOENT;
	}
	strncpy(de->name, to, DIR_NAME);
	return 0;
}

// TODO: Not yet
int storage_set_time(const char* path, const struct timespec ts[2]) {
	return -1;
}

// Returns an slist of all the items in a directory
slist* storage_list(const char* path) {
	return directory_list(path);	// s_split(path, "/");
}





