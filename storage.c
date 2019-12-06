#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "pages.h"
#include "util.h"

// initialization storage
void storage_init(const char *path) {
	pages_init(path);
	storage_directory_mk("/");
}

// makes directory
int storage_directory_mk(const char *path) {
	puts("does this make a directory?");
	int tmp = storage_file_mk(path, 040777);
	file_node *node = pages_fetch_node_with_num(tmp);
	strcpy(node->path, path);
	if (node->count == 0) {
		int writer = 0;
		storage_write_data(path, &writer, 4, 0); // 4 size of int
	}
	return tmp;
}

// reads all files from a directory into a buffer
int storage_directory_read(const char *path, void *buf, fuse_fill_dir_t filler) {
	return pages_read_inodes(path, buf, filler);
}

// gets the stats - fixed the mode here that fixed the mnt perms
int storage_stat(const char *path, struct stat *st) {
	printf("%s\n", path);
	file_node *tmp = pages_fetch_node(path);
	if (tmp == 0) {
		return -1;
	} else {
		// make sure everything is 0'd first
		memset((void*) st, 0, sizeof(struct stat));
		// then set stats
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_size = tmp->size;
		st->st_mode = tmp->mode;
		printf("refs: %d\n", tmp->refs);
		st->st_nlink = tmp->refs + 1;
		st->st_ino = tmp->node_num;
		st->st_atime = tmp->atime;
		st->st_ctime = tmp->ctime;
		st->st_mtime = tmp->mtime;
		return 0;
	}
}

// checks whether or not the storage contains the path
int storage_contains(const char *path) {
	file_node *node = pages_fetch_node(path);
	return (node == 0) ? -1 : 0;
}

// makes a file
int storage_file_mk(const char *path, mode_t mode) {
	file_node *node = pages_fetch_node(path);
	if (node != 0) {
		puts("not here");
		// basically, this is already a file, and it just returns the
		// node number
		return node->node_num;
	} else {
		int tmp = pages_fetch_empty(); // number at empty node
		node = pages_fetch_node_with_num(tmp);
		node->node_num = tmp;
		node->mode = mode;
		node->refs = 0;
		time_t now = time(NULL);
		node->atime = now;
		node->ctime = now;
		node->mtime = now;
		strcpy(node->path, path);
		if (!streq("/", path)) {
			printf("%s\n", path);
			int ii = strlen(path);
			for (; ii >= 0; --ii) {
				if (path[ii] == '/') {
					break;
				}
			}
			char* breadcrumbs = malloc(64);
			strncpy(breadcrumbs, path, ii);
			
			printf("%s\n", breadcrumbs);	
			puts("bruh moment");
			pages_add_file_dir("/", path);
			/*
			if (!strlen(breadcrumbs)) {
				pages_add_file_dir("/", path);
			}
			else {
				pages_add_file_dir(breadcrumbs, path);
			}*/
		}
		return tmp;
	}
}

// renames a file and updates accessed timestamp
int storage_file_rename(const char* path, const char* new) {
	file_node* node = pages_fetch_node(path);
	node->mtime = time(NULL);
	// literally just copy the path over from the old.
	strcpy(node->path, new); // do i need 0ing.
	// these aren't sys calls, so ret is just 0
	return 0;
}

// first four args of nufs_read, very similar to write data too
int
storage_fetch_data(const char *path, char* buf, size_t size, off_t offset) {
	file_node* node = pages_fetch_node(path);
	node->atime = time(NULL);
	if (node == 0 || offset < 0) {
		return -1;
	} else if (size == 0) {
		return 0;
	} else {
		// copies data of size into buff.
		void *pageTemp = pages_get_page(node->ptr[0]);
		pageTemp += offset;
		memcpy((void*) buf, pageTemp, size);
		return size;
	}
}

// writes the data to the storage by a buffer
int
storage_write_data(const char *path, const void* buf, size_t size, off_t offset) {
	file_node* node = pages_fetch_node(path);
	node->mtime = time(NULL);
	if (node == 0 || offset < 0) {
		return -1;
	} else if (size == 0) {
		return 0;
	} else if (node->count == 0) {
		pages_give_page(node); // same as above for the checkers, except this line.
	}
	void *pageTemp = pages_get_page(node->ptr[0]);
	pageTemp += offset;
	memcpy(pageTemp, buf, size);
	// increase the size after in case you wrote too much
	int sizeTotal = size + offset;
	node->size = max(size, sizeTotal);
	return size;
}

// removes a file from the storage
int storage_file_rm(const char *path) {
	file_node *node = pages_fetch_node(path);
	if (node == 0) {
		return -1;
	} else {
		int num = node->node_num;
		pages_free_node(node);
		file_node *dir = pages_fetch_node("/");
		pages_remove_node_dir(dir, num);
		return 0;
	}
}

// this function can literaly just set all the pointers from target to link
// and copies some other basic stuff.
// creates a link
int storage_link(const char *from_path, const char *link_path) {
	file_node *from_node = pages_fetch_node(from_path);
//	file_node* link_node = pages_fetch_node(link_path);
	int node_num = pages_fetch_empty();
	file_node *link_node = pages_fetch_node_with_num(node_num);

	// copy all the data over to strcpy and all the pointers
	strcpy(link_node->path, link_path);
	link_node->mode = from_node->mode;
	link_node->node_num = node_num;
	link_node->refs = 0;
	link_node->size = from_node->size;

	link_node->count = from_node->count;
	// in case something messes up, dont use link_node->count, use node->count
	// this ensures that I'm setting something at every point i guess.
	for (int ii = 0; ii < link_node->count; ++ii) {
		link_node->ptr[ii] = from_node->ptr[ii];
	}

	from_node->refs++;
	link_node->refs++;
	pages_add_file_dir("/", link_path);
	return node_num;
}

// changes mode for a file
int storage_chmod(const char* path, mode_t mode) {
	file_node* node = pages_fetch_node(path);
	if (!node) {
		return -ENOENT;
	}
	node->mtime = time(NULL);
	node->mode = mode;
	return 0;
}

// creates a symlink by storing the path to the file being linked to
int storage_symlink(const char* linkname, const char* path) {
	file_node* from = pages_fetch_node(path);	// Make sure this doesn't
	if (from) {
		return -EEXIST;
	}
	
	int rv = storage_file_mk(path, 0120777);
	if (rv < 0) {
		return -ENOSPC;
	}
	from = pages_fetch_node(path);	// This should exist now
	strncpy(pages_get_page(from->ptr[0]), linkname, 4096);
	return 0;
}

// fills the buffer with the name of the file being linked to
int storage_readlink(const char *from, char* buf, size_t size) {
	file_node* linkNode = pages_fetch_node(from);
	if (!linkNode) {
		return -ENOENT;
	}

	if (linkNode->mode & 0120000) {
		char* newPath = (char*)pages_get_page(linkNode->ptr[0]);
		printf("name newPath: %s", newPath);
		/*printf("%s, %d\n", newPath, size);
		storage_fetch_data(newPath, buf, size, 0);*/
		strncpy(buf, newPath, size);
	}
	else {
		// TODO: Check this error for permissions
		return -1;
	}

	return 0;
}

// updates the access and modify times to current
int storage_utimens(const char* path, const struct timespec ts[2]) {
	file_node* node = pages_fetch_node(path);
	if (!node) {
		return -ENOENT;
	}
	// Modify access and modify time, but not creation time
	node->atime = ts[0].tv_sec;
	node->mtime = ts[1].tv_sec;
	return 0;
}
