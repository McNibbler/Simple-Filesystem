// bitmap implementation

#include "bitmap.h"
#include <stdio.h>
#include <assert.h>

// Masking techniques inspired by user dbush on SO
// stackoverflow.com/questions/52742756/mapping-a-number-to-bit-position-in-c

// Returns if a bitmap value at an index is true or false
int bitmap_get(void* bm, int ii) {
	// Return true because it means you can not use this space. Perhaps I should make
	// it crash here instead idk
	assert(ii>=0);
	int byteIndex = ii / 8;
	int bitIndex = ii % 8;
	printf("%d: %d\n", ii, ((((char*)bm)[byteIndex] & (1 << bitIndex)) != 0));
	return ((((char*)bm)[byteIndex] & (1 << bitIndex)) != 0);
}

// Sets a bit in the bitmap at a given index to 0 or 1
void bitmap_put(void* bm, int ii, int vv) {
	// Do nothing for invalid. Perhaps I should make
	// it crash here instead idk
	assert(ii >= 0);
	assert(vv == 1 || vv == 0);
	int byteIndex = ii / 8;
	int bitIndex = ii % 8;
	if (vv) {
		((char*)bm)[byteIndex] |= (1 << bitIndex);
	}
	else {
		((char*)bm)[byteIndex] &= ((1 << bitIndex) ^ 0xff);
	}
}

// Prints the items in a bitmap for debugging
void bitmap_print(void* bm, int size) {
	int bits = size * 8;
	for (int ii = 0; ii < bits; ii++) {
		printf("%d\n", bitmap_get(bm, ii));
	}
}
