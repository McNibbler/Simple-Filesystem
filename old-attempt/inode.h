// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "pages.h"

#define DIR_PTRS 10

//TODO: Note - try mode_t and size_t
typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[DIR_PTRS]; // direct pointers
	// WARNING: NOT AN ACTUAL MEMORY POINTER
    int iptr; // single indirect pointer
} inode;

void inode_initialize();
void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
void free_inode(int inum);	// The hint didn't have an inodenum??
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);

#endif
