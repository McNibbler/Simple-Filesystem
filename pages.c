// based on cs3650 starter code huh

#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "pages.h"
#include "util.h"
#include "structs.h"


const int PAGE_COUNT = 256;
const int NUFS_SIZE  = 4096 * 256; // 1MB

static int nodes_count = 75;

static int   pages_fd   = -1;
static void* pages_base =  0;

static char* bitmap_node;
static char* bitmap_block;
static file_node* file_nodes;
static void* blocks;

int
pages_read_inodes(const char* path, void* buf, fuse_fill_dir_t filler)
{
    file_node* node = pages_fetch_node(path);
    int *page = pages_get_page(node->ptr[0]);
    int entries = page[0];

    for (int i = 0; i < entries; i++) {
        int tmp = page[i + 1];
        struct stat st;
        // 0s everything, in case
        memset(&st, 0, sizeof(struct stat));
        st.st_uid  = getuid();
        st.st_gid = getgid();
        st.st_mode = file_nodes[tmp].mode;
        st.st_size = file_nodes[tmp].size;
        st.st_ino = tmp; // is this necessary? i put it in to save me from
        // corrupted files because my vm crashed once, but commenting out
        // seems fine
        void* point = &(file_nodes[tmp].path);
        filler(buf, point + 1, &st, 0);

    }

    return 0;
}

void
pages_init(const char* path)
{
    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    // set bitmaps and set inodes
    void* curr = pages_base;
    // insert the bitmap for nodes at the base
    bitmap_node = curr;
    curr += nodes_count; // bitmap so everything is 1 byte
    // insert the bitmap for blocks at the curr base after the current node count
    bitmap_block = curr;
    curr += PAGE_COUNT; // bitmap so everything is 1 byte
    // file nodes go after the bitmaps.
    file_nodes = curr;
    // thrust the blocks in somewhere after that
    // ask about this...is there an ideal size?
    blocks = pages_base + 81920;

    // this is starter code that i'm keeping in
//    void* pbm = get_pages_bitmap();
//    bitmap_put(pbm, 0, 1);
}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

//starter code that i didn't use
//void*
//get_pages_bitmap()
//{
//    return pages_get_page(0);
//}
//
//void*
//get_inode_bitmap()
//{
//    uint8_t* page = pages_get_page(0);
//    return (void*)(page + 32);
//}

file_node*
pages_fetch_node(const char* path) {
	for (int ii = 0; ii < nodes_count; ++ii) {
		if (bitmap_node[ii] == 1 && streq(file_nodes[ii].path, path)) {
			return &(file_nodes[ii]);
		}
	}
	return 0;

}

void
pages_free_node(file_node* node) {
	// frees the node
	bitmap_node[node->node_num] = 0;
	// frees all the blocks that are pointed to by node
	for (int ii = 0; ii < node->count; ++ii) {
		bitmap_block[node->ptr[ii]] = 0;
	}
}

int
pages_fetch_empty() {
	// the get the first empty node
	for (int ii = 0; ii < PAGE_COUNT; ++ii) {
		if (bitmap_node[ii] == 0) {
			bitmap_node[ii] = 1;
			return ii;
		}
	}
	return -1;
}

file_node* pages_fetch_node_with_num(int ii) {
	return &(file_nodes[ii]);
}

int
pages_give_page(file_node* node)
{
	int counter = node->count;
	node->count++;
    int empty = pages_fetch_empty();
    node->ptr[counter] = empty;
    return empty;
}

void
pages_remove_node_dir(file_node* dir, int num) {
	int* dirs = pages_get_page(dir->ptr[0]);
	int sizeTemp = dirs[0];
	int subEnt = 0;
	if (dirs[sizeTemp] == num) {
		subEnt++;
	} else {
		for (int ii = 0; ii < sizeTemp; ++ii) {
			if (dirs[ii] == num) {
				dirs[ii] = dirs[sizeTemp];
				subEnt++;
				break;
			}
		}
	}
	if (subEnt != 0) {
		dirs[0]--;
	}
}

void
pages_add_file_dir(const char* dir, const char* file) {
	file_node* dirTemp = pages_fetch_node(dir);
	// get the int array for the dir
	int* data = pages_get_page(dirTemp->ptr[0]);
	file_node* fileTemp = pages_fetch_node(file);
	// set the node ID to the data of the dir, at its size + 1
	data[++data[0]] = fileTemp->node_num;
}
