#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "pages.h"
#include "util.h"

void
storage_init(const char* path)
{
    pages_init(path);
    storage_directory_mk("/");
}

int
storage_directory_mk(const char* path) {
	puts("does this make a directory?");
	int tmp = storage_file_mk(path, 040777);
	file_node* node = pages_fetch_node_with_num(tmp);
	strcpy(node->path, path);
	if (node->count == 0) {
		int writer = 0;
		storage_write_data(path, &writer, 4, 0); // 4 size of int
	}
	return tmp;
}

int
storage_directory_read(const char* path, void* buf, fuse_fill_dir_t filler) {
	return pages_read_inodes(path, buf, filler);
}

// gets the stats - fixed the mode here that fixed the mnt perms
int storage_stat(const char* path, struct stat* st) {
	printf("%s\n", path);
	file_node* tmp = pages_fetch_node(path);
	if (tmp == 0) {
		puts("fucking kill me");
		return -1;
//		return 0;
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
		return 0;
	}
}

int storage_contains(const char* path) {
	file_node* node = pages_fetch_node(path);
	return (node == 0) ? -1 : 0;
}

int
storage_file_mk(const char* path, mode_t mode) {
	file_node* node = pages_fetch_node(path);
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
		strcpy(node->path, path);
		if (!streq("/", path)) {
			printf("%s\n", path);
			pages_add_file_dir("/", path);
		}
		puts("i'm pretty sure i'm here");
		return tmp;
	}

}

int storage_file_rename(const char* path, const char* new) {
	file_node* node = pages_fetch_node(path);
	// literally just copy the path over from the old
	strcpy(node->path, new); // do i need 0ing.
	// these aren't sys calls, so ret is just 0
	return 0;
}

// first four args of nufs_read, very similar to write data too
int
storage_fetch_data(const char *path, char* buf, size_t size, off_t offset) {
	file_node* node = pages_fetch_node(path);
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

int
storage_write_data(const char *path, const void* buf, size_t size, off_t offset) {
	file_node* node = pages_fetch_node(path);
	puts("here in write data");
	if (node == 0 || offset < 0) {
		return -1;
	} else if (size == 0) {
		return 0;
	} else if (node->count == 0) {
		puts("here should be for empty");
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

int
storage_file_rm(const char* path) {
	file_node* node = pages_fetch_node(path);
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
int
storage_link(const char* from_path, const char* link_path) {
	file_node* from_node = pages_fetch_node(from_path);
//	file_node* link_node = pages_fetch_node(link_path);
	int node_num = pages_fetch_empty();
	file_node* link_node = pages_fetch_node_with_num(node_num);

	// copy all the data over to strcpy and all the pointers
	strcpy(link_node->path, link_path);
	link_node->mode = from_node->mode;
	link_node->node_num = node_num;
	link_node->refs = 0;

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
