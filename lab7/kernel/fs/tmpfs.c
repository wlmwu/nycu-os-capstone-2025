#include "tmpfs.h"
#include "fs.h"
#include "list.h"
#include "utils.h"
#include "errno.h"
#include "slab.h"
#include <stddef.h>

struct tmpfs_inode;

typedef struct tmpfs_dentry {
    char name[FS_MAX_COMPONENT_LEN + 1];    // Name + '\0'
    struct tmpfs_inode *inode;
    struct list_head sibling;               // List node to link all nodes under the same parent
} tmpfs_dentry_t;

typedef enum {
    INODE_TYPE_FILE,
    INODE_TYPE_DIRECTORY
} tmpfs_inode_type_t;

typedef struct tmpfs_inode {
    tmpfs_inode_type_t type;
    struct vnode *vnptr;                    // Point to the vnode which points to this struct
    union {
        // INODE_TYPE_DIRECTORY
        struct {
            struct list_head subdirs;       // List head of subdirs (children)
            size_t num_entries;
            struct tmpfs_inode *parent;      // Parent directory (root inode points to itself)
        } tn_dir;
        
        // INODE_TYPE_FILE
        struct {
            char data[FS_MAX_FILE_SIZE];
            size_t filesize;
        } tn_file;

    } tn_content;
} tmpfs_inode_t;


static struct vnode_operations tmpfs_vops = {
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir
};

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    struct vnode *root = kmalloc(sizeof(struct vnode));
    root->v_ops = &tmpfs_vops;
    root->f_ops = NULL;
    root->mount = NULL;

    tmpfs_inode_t *inode = kmalloc(sizeof(tmpfs_inode_t));
    inode->type = INODE_TYPE_DIRECTORY;
    inode->vnptr = root;
    INIT_LIST_HEAD(&inode->tn_content.tn_dir.subdirs);
    inode->tn_content.tn_dir.num_entries = 0;
    inode->tn_content.tn_dir.parent = inode;

    root->mount = NULL;
    root->internal = inode;
    
    mount->root = root;
    mount->fs = fs;

    return 0;
}

static tmpfs_dentry_t* tmpfs_create_dentry(tmpfs_inode_t *parent_inode, const char *name, tmpfs_inode_type_t type) {
    struct vnode *new_vn = kmalloc(sizeof(struct vnode));
    tmpfs_inode_t *new_in = kmalloc(sizeof(tmpfs_inode_t));
    tmpfs_dentry_t *new_den = kmalloc(sizeof(tmpfs_dentry_t));

    memset(new_den->name, 0, sizeof(new_den->name));
    memcpy(new_den->name, name, strlen(name));
    new_den->inode = new_in;
    list_add_tail(&new_den->sibling, &parent_inode->tn_content.tn_dir.subdirs);
    ++parent_inode->tn_content.tn_dir.num_entries;

    new_in->type = type;
    new_in->vnptr = new_vn;
    if (type == INODE_TYPE_DIRECTORY) {
        INIT_LIST_HEAD(&new_in->tn_content.tn_dir.subdirs);
        new_in->tn_content.tn_dir.num_entries = 0;
        new_in->tn_content.tn_dir.parent = parent_inode;
    } else if (type == INODE_TYPE_FILE) {
        new_in->tn_content.tn_file.filesize = 0;
    }
    
    new_vn->v_ops = parent_inode->vnptr->v_ops;
    new_vn->f_ops = kmalloc(sizeof(struct file_operations));
    new_vn->f_ops->read = tmpfs_read;
    new_vn->f_ops->write = tmpfs_write;
    new_vn->f_ops->open = tmpfs_open;
    new_vn->f_ops->close = tmpfs_close;
    new_vn->f_ops->lseek64 = tmpfs_lseek64;
    new_vn->mount = NULL;
    new_vn->internal = new_in;

    return new_den;
}

/* vnode ops */

int tmpfs_lookup(struct vnode *dnode, struct vnode **target, const char *name) {
    if (strcmp(name, ".") == 0) {
        *target = dnode;
        return 0;
    } else if (strcmp(name, "..") == 0) {
        *target = ((tmpfs_inode_t*)(dnode->internal))->tn_content.tn_dir.parent->vnptr;
        return 0;
    }

    tmpfs_inode_t *parent_inode = dnode->internal;
    tmpfs_dentry_t *pos, *tmp;
    list_for_each_entry_safe(pos, tmp, &parent_inode->tn_content.tn_dir.subdirs, sibling) {
        if (strcmp(pos->name, name) == 0) {
            *target = pos->inode->vnptr;
            return 0;
        }
    }
    return -ENOENT;
}

int tmpfs_create(struct vnode *dnode, struct vnode **target, const char *name) {
    if (strlen(name) > FS_MAX_COMPONENT_LEN) return -ENAMETOOLONG;

    tmpfs_inode_t *parent_inode = dnode->internal;
    tmpfs_dentry_t *pos, *tmp;
    list_for_each_entry_safe(pos, tmp, &parent_inode->tn_content.tn_dir.subdirs, sibling) {
        if (strcmp(pos->name, name) == 0) return -EEXIST;
    }
    
    tmpfs_dentry_t *new_den = tmpfs_create_dentry(parent_inode, name, INODE_TYPE_FILE);    
    *target = new_den->inode->vnptr;

    return 0;
}

int tmpfs_mkdir(struct vnode *dnode, struct vnode **target, const char *name) {
    if (strlen(name) > FS_MAX_COMPONENT_LEN) return -ENAMETOOLONG;
    
    tmpfs_inode_t *parent_inode = dnode->internal;
    tmpfs_dentry_t *pos, *tmp;
    list_for_each_entry_safe(pos, tmp, &parent_inode->tn_content.tn_dir.subdirs, sibling) {
        if (strcmp(pos->name, name) == 0) return -EEXIST;
    }

    tmpfs_dentry_t *new_den = tmpfs_create_dentry(parent_inode, name, INODE_TYPE_DIRECTORY);    
    *target = new_den->inode->vnptr;

    return 0;
}

/* file ops */

int tmpfs_read(struct file *file, void *buf, size_t count) {
    tmpfs_inode_t *finode = file->vnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    int64_t len = MIN((int)count, (int)(finode->tn_content.tn_file.filesize - file->f_pos));
    if (len <= 0) return 0;

    memcpy(buf, finode->tn_content.tn_file.data, len);
    file->f_pos += len;

    return len;
}

int tmpfs_write(struct file *file, const void *buf, size_t count) {
    tmpfs_inode_t *finode = file->vnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    int64_t len = MIN((int)count, (int)(FS_MAX_FILE_SIZE - file->f_pos));
    if (len <= 0) return -ENOSPC;

    memcpy(finode->tn_content.tn_file.data, buf, len);
    file->f_pos += len;
    finode->tn_content.tn_file.filesize = MAX(file->f_pos, finode->tn_content.tn_file.filesize);

    return len;
}

int tmpfs_open(struct vnode *fvnode, struct file **target) {
    tmpfs_inode_t *finode = fvnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    struct file *file = kmalloc(sizeof(struct file));
    file->vnode = fvnode;
    file->f_pos = 0;
    file->f_ops = fvnode->f_ops;
    file->flags = 0;                // Unused
    *target = file;
    return 0;
}

int tmpfs_close(struct file *file) {
    tmpfs_inode_t *finode = file->vnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    kfree(file);
    return 0;
}

long tmpfs_lseek64(struct file *file, long offset, int whence) {
    return -ENOSYS;
}