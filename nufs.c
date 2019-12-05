// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS  64
#include <fuse.h>
#include "structs.h"
#include "storage.h"
#include "util.h"
#include "pages.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    int rv = 0;
    rv = storage_contains(path);
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
	int rv = storage_stat(path, st);
	printf("getattr(%s) -> %d\n", path, rv);
	return (rv == -1) ? -ENOENT : 0;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;
    int rv = nufs_getattr(path, &st);
    assert(rv == 0);

    filler(buf, ".", &st, 0);

//    rv = nufs_getattr("/hello.txt", &st);
//    assert(rv == 0);
//    filler(buf, "hello.txt", &st, 0);

    storage_directory_read(path, buf, filler);
    printf("readdir(%s) -> %d\n", path, rv);
    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	if (streq(path, "/cc")) {
	    file_node* node = pages_fetch_node("/");
	    int *page = pages_get_page(node->ptr[0]);
        printf("page[5] right now: %d", page[5]);
	}
    int rv = -1;
    if (mode & 040000) {
    	rv = storage_directory_mk(path);
    } else {
    	puts("entered file");





    	rv = storage_file_mk(path, mode);
    }
    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    // file make is actualy returning the inode value upon return
    // i have to ignore rv here because otherwise it breaks
	if (streq(path, "/cc")) {
	    file_node* node = pages_fetch_node("/");
	    int *page = pages_get_page(node->ptr[0]);
        printf("page[5] right now: %d", page[5]);
	}
    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
// not relevant yet
int
nufs_mkdir(const char *path, mode_t mode)
{
    int rv = nufs_mknod(path, mode | 040000, 0);
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

// same as rmdir
int
nufs_unlink(const char *path)
{
    int rv = -1;
    rv = storage_file_rm(path);
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv == -1 ? -ENOENT : 0;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = -1;
    puts("pooping");
    rv = storage_link(from, to);
    printf("link(%s => %s) -> %d\n", from, to, rv);
	return (rv == -1) ? -ENOENT : 0;
}

// same as below
int
nufs_rmdir(const char *path)
{
    int rv = -1;
    rv = storage_file_rm(path);
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv == -1 ? -ENOENT : 0;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    int rv = -1;
    rv = storage_file_rename(from, to);
    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

// not relevant yet
int
nufs_chmod(const char *path, mode_t mode)
{
    int rv = storage_chmod(path, mode);
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

// not relevant yet
int
nufs_truncate(const char *path, off_t size)
{
    int rv = 0;
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// this is doing something, but I dont need to change it
// just changed rv to 0
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    int rv = 0;
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = 6;
    rv = storage_fetch_data(path, buf, size, offset);
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = -1;
    char* bufTemp = (char*)buf; // i'm confused by this line and how it still works
    // even though i made a change to write data but oh well
    rv = storage_write_data(path, bufTemp, size, offset);
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Update the timestamps on a file or directory.
// not relevant yet
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = 0;
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
// not relevant yet
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = 0;
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    printf("TODO: mount %s as data file\n", argv[--argc]);
    storage_init(argv[argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

