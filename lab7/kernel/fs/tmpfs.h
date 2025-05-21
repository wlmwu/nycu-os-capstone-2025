#ifndef TMPFS_H_
#define TMPFS_H_

#include "vfs.h"
#include "fs.h"
#include "device.h"

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount);

/* vnode ops */

int tmpfs_lookup(struct vnode *dnode, struct vnode **target, const char *name);
int tmpfs_create(struct vnode *dnode, struct vnode **target, const char *name);
int tmpfs_mkdir(struct vnode *dnode, struct vnode **target, const char *name);
int tmpfs_mknod(struct vnode *dnode, struct vnode **target, const char *name, dev_t dev);

/* file ops */

int tmpfs_read(struct file *file, void *buf, size_t count);
int tmpfs_write(struct file *file, const void *buf, size_t count);
int tmpfs_open(struct vnode *fvnode, struct file **target);
int tmpfs_close(struct file *file);
long tmpfs_lseek64(struct file *file, long offset, int whence);

#endif // TMPFS_H_