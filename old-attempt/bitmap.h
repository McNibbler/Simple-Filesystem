// based on cs3650 starter code

#ifndef BITMAP_H
#define BITMAP_H

// Gets a boolean describing the used state of the bit at bit index ii
int bitmap_get(void* bm, int ii);

// Puts boolean value vv at bit index ii in the bitmap
void bitmap_put(void* bm, int ii, int vv);

// Prints the contents of the bitmap (I think size is in bytes??)
void bitmap_print(void* bm, int size);

#endif
