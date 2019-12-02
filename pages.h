// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "structs.h"


void pages_init(const char* path);
void pages_free();
void* pages_get_page(int pnum);
void* get_pages_bitmap();
void* get_inode_bitmap();
int alloc_page();
void free_page(int pnum);
int pages_read_inodes(const char* path, void* buf, fuse_fill_dir_t filler);
file_node* pages_fetch_node(const char* path);
int pages_fetch_empty();
file_node* pages_fetch_node_with_num(int ii);
int pages_give_page(file_node* node);
void pages_free_node(file_node* node);
void pages_remove_node_dir(file_node* dir, int num);
void pages_add_file_dir(const char* dir, const char* file);

#endif
