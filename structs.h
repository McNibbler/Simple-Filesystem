#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <fuse.h>


typedef struct file_node {
    int refs;
    int size;
    int mode;
    int count; // count the number of blocks used for file
    int ptr[30];
    // iptr not necessary right now, but it was in starter code;
    // int iptr; setting ptr[10] is enough for 40k
    int node_num; // holds the st_ino, also helps me in my array
    time_t atime;	// Access
    time_t ctime;	// Create 
    time_t mtime;	// Modify
    char path[64]; // entire path of file
} file_node;

#endif
