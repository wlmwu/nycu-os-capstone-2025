#include "initramfs.h"
#include "fs.h"
#include "vfs.h"
#include "cpio.h"
#include "slab.h"
#include "list.h"
#include "utils.h"
#include "errno.h"

typedef struct initramfs_inode initramfs_inode_t;
typedef struct initramfs_dentry initramfs_dentry_t;

typedef enum {
    INODE_TYPE_FILE,
    INODE_TYPE_DIRECTORY
} initramfs_inode_type_t;

struct initramfs_dentry {
    char name[FS_MAX_COMPONENT_LEN + 1];    // Name + '\0'
    initramfs_inode_t *inode;
    struct list_head sibling;               // List node to link all nodes under the same parent
};

struct initramfs_inode {
    initramfs_inode_type_t type;
    struct vnode *vnptr;                    // Point to the vnode which points to this struct
    union {
        // INODE_TYPE_DIRECTORY
        struct {
            struct list_head subdirs;       // List head of subdirs (children)
            size_t num_entries;
            initramfs_inode_t *parent;      // Parent directory (root inode points to itself)
        } tn_dir;
        
        // INODE_TYPE_FILE
        struct {
            char *data;                     // Pointer of the file data in CPIO
            size_t filesize;
        } tn_file;

    } tn_content;
};


static int initramfs_lookup(fs_vnode_t *dnode, fs_vnode_t **target, const char *name) {
    if (strcmp(name, ".") == 0) {
        *target = dnode;
        return 0;
    } else if (strcmp(name, "..") == 0) {
        *target = ((initramfs_inode_t*)(dnode->internal))->tn_content.tn_dir.parent->vnptr;
        return 0;
    }

    initramfs_inode_t *parent_inode = dnode->internal;
    initramfs_dentry_t *pos, *tmp;
    list_for_each_entry_safe(pos, tmp, &parent_inode->tn_content.tn_dir.subdirs, sibling) {
        if (strcmp(pos->name, name) == 0) {
            *target = pos->inode->vnptr;
            return 0;
        }
    }
    return -ENOENT;
}

static int initramfs_create(fs_vnode_t *dnode, fs_vnode_t **target, const char *name) {
    return -EROFS;

}

static int initramfs_mkdir(fs_vnode_t *dnode, fs_vnode_t **target, const char *name) {
    return -EROFS;
}

static int initramfs_getattr(fs_vnode_t *vnode, fs_vattr_t *attr) {
    initramfs_inode_t *inode = vnode->internal;
    if (inode->type == INODE_TYPE_FILE) {
        attr->size = inode->tn_content.tn_file.filesize;
    } else if (inode->type == INODE_TYPE_DIRECTORY) {
        attr->size = inode->tn_content.tn_dir.num_entries;
    }
    return 0;
}


static int initramfs_read(fs_file_t *file, void *buf, size_t count) {
    initramfs_inode_t *finode = file->vnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    int64_t len = MIN((int)count, (int)(finode->tn_content.tn_file.filesize - file->f_pos));
    if (len <= 0) return 0;

    memcpy(buf, finode->tn_content.tn_file.data + file->f_pos, len);
    file->f_pos += len;

    return len;
}

static int initramfs_write(fs_file_t *file, const void *buf, size_t count) {
    return -EROFS;
}

static int initramfs_open(fs_vnode_t *fvnode, fs_file_t **target) {
    initramfs_inode_t *finode = fvnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    struct file *file = kmalloc(sizeof(struct file));
    file->vnode = fvnode;
    file->f_pos = 0;
    file->f_ops = fvnode->f_ops;
    file->flags = 0;                // Unused
    *target = file;
    return 0;
}

static int initramfs_close(fs_file_t *file) {
    initramfs_inode_t *finode = file->vnode->internal;
    if (finode->type == INODE_TYPE_DIRECTORY) return -EISDIR;

    kfree(file);
    return 0;
}

static long initramfs_lseek64(fs_file_t *file, long offset, int whence) {
    if (whence == SEEK_SET) {
        file->f_pos = offset;
        return offset;
    }
    return -EINVAL;
}


static struct vnode_operations initramfs_vops = {
    .lookup = initramfs_lookup,
    .create = initramfs_create,
    .mkdir = initramfs_mkdir,
    .getattr = initramfs_getattr,
};

static struct file_operations iniramfs_fops = {
    .open = initramfs_open,
    .close = initramfs_close,
    .read = initramfs_read,
    .write = initramfs_write,
    .lseek64 = initramfs_lseek64
};


static initramfs_dentry_t* initramfs_create_dentry(initramfs_inode_t *parent_inode, const char *name) {
    struct vnode *new_vn = kmalloc(sizeof(fs_vnode_t));
    initramfs_inode_t *new_in = kmalloc(sizeof(initramfs_inode_t));
    initramfs_dentry_t *new_den = kmalloc(sizeof(initramfs_dentry_t));

    memset(new_den->name, 0, sizeof(new_den->name));
    memcpy(new_den->name, name, strlen(name));
    new_den->inode = new_in;
    list_add_tail(&new_den->sibling, &parent_inode->tn_content.tn_dir.subdirs);
    ++parent_inode->tn_content.tn_dir.num_entries;

    new_in->vnptr = new_vn;
    
    new_vn->v_ops = parent_inode->vnptr->v_ops;
    new_vn->f_ops = &iniramfs_fops;
    new_vn->mount = NULL;
    new_vn->internal = new_in;

    return new_den;
}

static int build_fs(fs_vnode_t *root) {
    cpio_newc_header_t *hptr = cpio_get_start_file();
    while (hptr) {
        char *pathname;
        uint32_t filesize;
        char *filedata;
        hptr = cpio_get_file(hptr, &pathname, &filesize, &filedata);
        if (!pathname) break;
        
        fs_vnode_t *curr = root;
        char *path = strdup(pathname);
        char *tok = strtok(path, "/");

        char *last_slash = strrchr(path, '/');
        char *basename = last_slash + 1;

        while (tok) {
            fs_vnode_t *next;
            int retval = initramfs_lookup(curr, &next, tok);
            if (retval == -ENOENT) {
                initramfs_inode_t *parent_inode = curr->internal;
                initramfs_dentry_t *pos, *tmp;
                list_for_each_entry_safe(pos, tmp, &parent_inode->tn_content.tn_dir.subdirs, sibling) {
                    if (strcmp(pos->name, tok) == 0) return -EEXIST;
                }
                
                initramfs_dentry_t *new_den = initramfs_create_dentry(parent_inode, tok);    
                initramfs_inode_t *new_in = new_den->inode;
                if (filesize) {
                    new_in->type = INODE_TYPE_FILE;
                    new_in->tn_content.tn_file.data = filedata;
                    new_in->tn_content.tn_file.filesize = filesize;
                    filesize = 0;
                } else {
                    new_in->type = INODE_TYPE_DIRECTORY;
                    INIT_LIST_HEAD(&new_in->tn_content.tn_dir.subdirs);
                    new_in->tn_content.tn_dir.num_entries = 0;
                    new_in->tn_content.tn_dir.parent = parent_inode;
                }

                next = new_in->vnptr;
            
            } else if (retval != 0) {
                free(path);
                return retval;
            }

            curr = next;
            tok = strtok(NULL, "/");
        }

        free(path);
    }
    return 0;
}

static int initramfs_setup_mount(struct filesystem *fs, struct mount *mount) {
    struct vnode *root = kmalloc(sizeof(struct vnode));
    root->v_ops = &initramfs_vops;
    root->f_ops = NULL;
    root->mount = NULL;

    initramfs_inode_t *inode = kmalloc(sizeof(initramfs_inode_t));
    inode->type = INODE_TYPE_DIRECTORY;
    inode->vnptr = root;
    INIT_LIST_HEAD(&inode->tn_content.tn_dir.subdirs);
    inode->tn_content.tn_dir.num_entries = 0;
    inode->tn_content.tn_dir.parent = inode;

    root->mount = NULL;
    root->internal = inode;
    
    mount->root = root;
    mount->fs = fs;

    return build_fs(root);
}


static struct filesystem initramfs_fs = {
    .name = "initramfs",
    .setup_mount = initramfs_setup_mount
};

void initramfs_init() {
    fs_register(&initramfs_fs);
    fs_mount_t *initramfs_root = kmalloc(sizeof(fs_mount_t));
    fs_vnode_t *root = fs_get_root()->root;

    vfs_mkdir(root, "/initramfs/");    
    vfs_mount(root, "/initramfs/", "initramfs");
}
