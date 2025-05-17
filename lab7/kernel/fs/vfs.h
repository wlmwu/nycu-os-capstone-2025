#ifndef VFS_H_
#define VFS_H_

#include "fs.h"

struct vnode_operations {
    /**
     * @brief Looks up a child vnode by name within a directory vnode.
     *
     * @param dnode Pointer to the vnode structure of the parent directory.
     * @param target Pointer to a vnode pointer where the found child vnode will be stored.
     * @param name The name of the child vnode to look up.
     * @return 0 on success, or a negative error code if the child is not found.
     */
    int (*lookup)(struct vnode *dnode, struct vnode **target, const char *name);
    /**
     * @brief Creates a new file within a directory vnode.
     *
     * @param dnode Pointer to the vnode structure of the parent directory.
     * @param target Pointer to a vnode pointer where the new file's vnode will be stored.
     * @param name The name of the new file to create.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*create)(struct vnode *dnode, struct vnode **target, const char *name);
    /**
     * @brief Creates a new directory within a directory vnode.
     *
     * @param dnode Pointer to the vnode structure of the parent directory.
     * @param target Pointer to a vnode pointer where the new directory's vnode will be stored.
     * @param name The name of the new directory to create.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*mkdir)(struct vnode *dnode, struct vnode **target, const char *name);
};

struct vnode {
    struct mount *mount;
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    void *internal;                   // Stored inode defined by filesystems themselves
};

int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_read(struct file *file, void *buf, size_t count);
int vfs_write(struct file *file, const void *buf, size_t count);
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);

// https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/fcntl.h

#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_CREAT		00000100

#endif // VFS_H_
