#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <fuse.h>


typedef struct file_node {
//    int refs; starter code not needed rn
    int size;
    int mode;
    int count; // count the number of blocks used for file
    int ptr[10];
    // iptr not necessary right now, but it was in starter code;
    // int iptr;
    int node_num; // holds the st_ino, also helps me in my array
    char path[64]; // entire path of file
} file_node;

#endif
