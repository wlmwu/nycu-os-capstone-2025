#ifndef FS_H_
#define FS_H_

#include <stdint.h>
#include <stddef.h>

#define FS_MAX_FILE_SIZE        4096
#define FS_MAX_COMPONENT_LEN    31
#define FS_MAX_NUM_ENTRY        16

// Defined in vfs.h
typedef struct vnode fs_vnode_t;
struct vnode_operations;
typedef struct vnode_attr fs_vattr_t;

typedef struct mount fs_mount_t;
typedef struct file fs_file_t;
struct file_operations;
struct filesystem;

struct file_operations {
    /**
     * @brief Writes data to an open file.
     *
     * @param file Pointer to the file structure representing the open file.
     * @param buf Pointer to the buffer containing the data to write.
     * @param count Number of bytes to write from the buffer.
     * @return The number of bytes successfully written, or a negative error code on failure.
     */
    int (*write)(struct file *file, const void *buf, size_t count);
    /**
     * @brief Reads data from an open file.
     *
     * @param file Pointer to the file structure representing the open file.
     * @param buf Pointer to the buffer where the read data will be stored.
     * @param count Maximum number of bytes to read.
     * @return The number of bytes successfully read, 0 if end-of-file is reached, or a negative error code on failure.
     */
    int (*read)(struct file *file, void *buf, size_t count);
    /**
     * @brief Opens the file or directory represented by a vnode.
     *
     * @param fnode Pointer to the vnode structure of the file or directory to open.
     * @param target Pointer to a file pointer where the new file structure will be stored.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*open)(struct vnode *fnode, struct file **target);
    /**
     * @brief Closes an open file.
     *
     * @param file Pointer to the file structure representing the open file to close.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*close)(struct file *file);
    /**
     * @brief Changes the current file offset of an open file.
     *
     * @param file Pointer to the file structure of the open file.
     * @param offset The offset to move the file pointer by.
     * @param whence Specifies the reference point for the offset:
     * - SEEK_SET: beginning of the file.
     * - SEEK_CUR: current file pointer position.
     * - SEEK_END: end of the file.
     * @return The new file offset in bytes from the beginning of the file, or -1 on failure.
     */
    long (*lseek64)(struct file *file, long offset, int whence);
    /**
     * @brief Performs device-specific I/O control operations on an open file.
     *
     * This function allows user-space applications to send commands and data
     * to the underlying device driver for specialized operations that are
     * not covered by standard read/write operations.
     *
     * @param file Pointer to the file structure representing the open file.
     * @param cmd The command code to be executed. These commands are typically
     * defined by the device driver.
     * @param arg An optional argument, typically a pointer to a data structure
     * or an integer value, whose meaning depends on the `cmd`.
     * @return 0 on success, or a negative error code on failure.
     */
    int (*ioctl)(struct file *file, unsigned long cmd, void *arg);
};

struct file {
  struct vnode *vnode;
  size_t f_pos;
  struct file_operations *f_ops;
  int flags;
};

struct mount {
  struct vnode *root;       // Mounted FS root vnode; its 'mount' points back here if there is no mount stacking on it
  struct filesystem *fs;
  struct vnode *covered;    // Vnode where this FS is mounted on
};

struct filesystem {
  const char *name;
  int (*setup_mount)(struct filesystem*, struct mount*);
};

int fs_register(struct filesystem *fs);
struct filesystem* fs_get_filesystem(const char *fs_name);

void fs_init();

fs_vnode_t* fs_get_root();

// https://github.com/torvalds/linux/blob/master/include/uapi/linux/fs.h
 
#define SEEK_SET	0	/* seek relative to beginning of file */
#define SEEK_CUR	1	/* seek relative to current file position */
#define SEEK_END	2	/* seek relative to end of file */

#endif // FS_H_