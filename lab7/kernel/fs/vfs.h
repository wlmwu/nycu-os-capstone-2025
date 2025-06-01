#ifndef VFS_H_
#define VFS_H_

#include "fs.h"
#include "device.h"

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
    /**
     * @brief Creates a new special file (device node) within a directory vnode.
     *
     * @param dnode Pointer to the vnode structure of the parent directory.
     * @param target Pointer to a vnode pointer where the new special file's vnode will be stored.
     * @param name The name of the new special file to create.
     * @param dev The device number (combination of major and minor numbers).
     * @return 0 on success, or a negative error code on failure.
     */
    int (*mknod)(struct vnode *dnode, struct vnode **target, const char *name, uint32_t dev);
    /**
     * @brief Retrieves the attributes (metadata) of a vnode.
     *
     * This function is responsible for populating a `vfs_file_stat_t` structure
     * with various metadata about the vnode, such as file size, type, permissions,
     * timestamps, etc. It serves as the underlying operation for `stat()` and `fstat()`
     * system calls.
     *
     * @param vnode Pointer to the vnode structure whose attributes are to be retrieved.
     * @param stat Pointer to a `vfs_file_stat_t` structure where the attributes will be stored.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*getattr)(struct vnode *vnode, struct vnode_attr *attr);
};

struct vnode {
    struct mount *mount;              // Non-NULL if this vnode is a mount point or the root of a mounted FS
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    void *internal;                   // Stored inode defined by filesystems themselves
};

struct vnode_attr {
    size_t size;        // file size in bytes
};

int vfs_open(fs_vnode_t *start, const char *pathname, int flags, fs_file_t **target);
int vfs_close(fs_file_t *file);
int vfs_read(fs_file_t *file, void *buf, size_t count);
int vfs_write(fs_file_t *file, const void *buf, size_t count);
long vfs_lseek64(fs_file_t *file, long offset, int whence);
int vfs_ioctl(fs_file_t *file, unsigned long cmd, void *arg);
int vfs_lookup(fs_vnode_t *start, const char *pathname, fs_vnode_t **target);
int vfs_mkdir(fs_vnode_t *start, const char *pathname);
int vfs_mknod(fs_vnode_t *start, const char *pathname, dev_t dev);
int vfs_mount(fs_vnode_t *start, const char *target, const char *filesystem);
int vfs_getattr(fs_vnode_t *vnode, fs_vattr_t *attr);

// https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/fcntl.h

#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_CREAT		00000100

#endif // VFS_H_
