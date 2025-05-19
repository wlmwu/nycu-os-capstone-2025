#include "fs.h"
#include "slab.h"
#include "tmpfs.h"
#include "utils.h"

#define NUM_FS_MAX 16

static struct mount *rootfs;

static struct filesystem *filesystems[NUM_FS_MAX];

static struct filesystem tmpfs_fs = {
    .name = "tmpfs",
    .setup_mount = tmpfs_setup_mount
};

void fs_init() {
    fs_register(&tmpfs_fs);
    rootfs = kmalloc(sizeof(struct mount));
    tmpfs_fs.setup_mount(&tmpfs_fs, rootfs);
}

int fs_register(struct filesystem *fs) {
    for (int i = 0; i < NUM_FS_MAX; ++i) {
        if (!filesystems[i]) {
            filesystems[i] = fs;
            return 0;
        }
    }
    return -1;
}

struct filesystem* fs_get_filesystem(const char *fs_name) {
    for (int i = 0; i < NUM_FS_MAX; ++i) {
        if (strcmp(fs_name, filesystems[i]->name) == 0) {
            return filesystems[i];
        }
    }
    return NULL;
}

struct mount* fs_get_root() {
    return rootfs;
}