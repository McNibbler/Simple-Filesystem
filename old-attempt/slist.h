// based on cs3650 starter code

#ifndef SLIST_H
#define SLIST_H

typedef struct slist {
    char* data;
    int   refs;
    struct slist* next;
} slist;


// Interface Functions //

// Adds an slist node to the head of an slist
slist* s_cons(const char* text, slist* rest);

// Frees all the elements of an slist
void   s_free(slist* xs);

// Parses a string on a specified delimiting character to produce an slist
slist* s_split(const char* text, char delim);

#endif

