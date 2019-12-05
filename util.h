// based on cs3650 starter code

#ifndef UTIL_H
#define UTIL_H

#include <string.h>

static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

static int
min(int x, int y)
{
    return (x < y) ? x : y;
}

static int
max(int x, int y)
{
    return (x > y) ? x : y;
}

static int
clamp(int x, int v0, int v1)
{
    return max(v0, min(x, v1));
}

static int
bytes_to_pages(int bytes)
{
    int quo = bytes / 4096;
    int rem = bytes % 4096;
    if (rem == 0) {
        return quo;
    }
    else {
        return quo + 1;
    }
}

static void
join_to_path(char* buf, char* item)
{
    int nn = strlen(buf);
    if (buf[nn - 1] != '/') {
        strcat(buf, "/");
    }
    strcat(buf, item);
}

// checks if a path is in a directory indirectly
static int
is_in_directory(const char* directory, const char* path) {
	int strLen = strlen(directory);
	if (strlen(path) <= strLen + 1) { // u need like /a/a at least 2 more
		return 0;
	}
	for (int ii = 0; ii < strLen; ++ii) {
		if (directory[ii] != path[ii]) {
			return 0;
		}
	}
	if (path[strLen] == '/') {
		return 1;
	} else {
		return 0;
	}
}

// checks if a path is in a directory directly
static int
is_in_directory_directly(const char* directory, const char* path) {
	int strLen = strlen(directory);
	int strLenPath = strlen(path);
	if (strLenPath <= strLen + 1) { // u need like /a/a at least 2 more
		return 0;
	}
	for (int ii = 0; ii < strLen; ++ii) {
		if (directory[ii] != path[ii]) {
			return 0;
		}
	}
	if (path[strLen] != '/') {
		return 0;
	} else {
		for (int ii = strLen + 1; ii < strLenPath; ++ii) {
			if (directory[ii] == '/') {
				return 0;
			}
		}
		return 1;
	}
}

// checks if a path is in a directory directly
static int
is_in_root_directly(const char* path) {
	for (int ii = 1; ii < strlen(path); ++ii) {
		if (path[ii] == '/') {
			return 0;
		}
	}
	return 1;
}

#endif
