#include "vfs.h"
#include "fs.h"
#include "slab.h"
#include "utils.h"
#include "errno.h"
#include "tmpfs.h"
#include "device.h"

int vfs_lookup(struct vnode *start, const char *pathname, struct vnode **target) {
    struct vnode *root = fs_get_root();
    
    struct vnode *curr;
    char *path, *saveptr;
    if (strncmp(pathname, "/", strlen("/")) == 0) {
        curr = root;
        path = strdup(pathname + 1);        // Skip leading '/'
    } else {
        curr = start;
        path = strdup(pathname);
    }

    char *tok = strtok_r(path, "/", &saveptr);
    while (tok) {
        if (strcmp(tok, "..") == 0 && curr->mount && curr->mount->root == curr) {   // `curr` is the root vnode of a mounted file system
            curr = curr->mount->covered;
        }

        struct vnode *next;
        int retval = curr->v_ops->lookup(curr, &next, tok);
        if (retval != 0) {
            free(path);
            return retval;
        }
        curr = next;
        while (curr->mount && curr->mount->root != curr) {
            curr = curr->mount->root;
        }
        tok = strtok_r(NULL, "/", &saveptr);
    }
    free(path);
    *target = curr;
    return 0;
}

int vfs_open(struct vnode *start, const char *pathname, int flags, struct file **target) {
    struct vnode *vn;
    int retval = vfs_lookup(start, pathname, &vn);
    if (retval == -ENOENT && (flags & O_CREAT)) {
        char *path = strdup(pathname);
        char *last_slash = strrchr(path, '/');
        if (!last_slash) {
            free(path);
            return -EINVAL;
        }

        *last_slash = '\0';
        char *basename = last_slash + 1;
        struct vnode *parent;
        if (vfs_lookup(start, *path ? path : "/", &parent) != 0) {
            free(path);
            return -ENOENT;
        }

        struct vnode *newfile;
        retval = parent->v_ops->create(parent, &newfile, basename);
        
        free(path);

        if (retval != 0) return retval;
        else vn = newfile;

    } else if (retval != 0) {
        return retval;
    }

    return vn->f_ops->open(vn, target);
}

int vfs_close(struct file *file) {
    return file->f_ops->close(file);
}

int vfs_read(struct file *file, void *buf, size_t count) {
    return file->f_ops->read(file, buf, count);
}

int vfs_write(struct file *file, const void *buf, size_t count) {
    return file->f_ops->write(file, buf, count);
}

long vfs_lseek64(struct file *file, long offset, int whence) {
    return file->f_ops->lseek64(file, offset, whence);
}

int vfs_ioctl(struct file *file, unsigned long cmd, void *arg) {
    return file->f_ops->ioctl(file, cmd, arg);
}

int vfs_mkdir(struct vnode *start, const char *pathname) {
    char *path = strdup(pathname);
    if (path[strlen(path) - 1] == '/') path[strlen(path) - 1] = '\0';

    char *last_slash = strrchr(path, '/');
    if (!last_slash) {
        free(path);
        return -EINVAL;
    }

    char *dirname = (last_slash == path) ? "/" : path;
    *last_slash = '\0';
    char *basename = last_slash + 1;

    struct vnode *parent;
    int retval = vfs_lookup(start, dirname, &parent);
    if (retval != 0) {
        free(path);
        return retval;
    }

    struct vnode *child;
    retval = parent->v_ops->mkdir(parent, &child, basename);
    
    free(path);
    return retval;
}

int vfs_mknod(fs_vnode_t *start, const char *pathname, dev_t dev) {
    char *path = strdup(pathname);
    if (path[strlen(path) - 1] == '/') path[strlen(path) - 1] = '\0';

    char *last_slash = strrchr(path, '/');
    if (!last_slash) {
        free(path);
        return -EINVAL;
    }

    char *dirname = (last_slash == path) ? "/" : path;
    *last_slash = '\0';
    char *basename = last_slash + 1;

    struct vnode *parent;
    int retval = vfs_lookup(start, dirname, &parent);
    if (retval != 0) {
        free(path);
        return retval;
    }

    struct vnode *child;
    retval = parent->v_ops->mknod(parent, &child, basename, dev);
    
    free(path);
    return retval;
}

int vfs_mount(struct vnode *start, const char *target, const char *filesystem) {
    struct vnode *mount_point;
    int retval = vfs_lookup(start, target, &mount_point);
    if (retval != 0) return retval;

    struct filesystem *fs = fs_get_filesystem(filesystem);
    if (!fs) return -ENOENT;

    struct mount *new_mount = kmalloc(sizeof(struct mount));
    retval = fs->setup_mount(fs, new_mount);
    if (retval != 0) {
        kfree(new_mount);
        return retval;
    }

    new_mount->covered = mount_point;
    while (mount_point->mount && mount_point->mount->root != mount_point) {
        mount_point = mount_point->mount->root;
    }
    mount_point->mount = new_mount;
    new_mount->root->mount = new_mount;
    
    return 0;
}

int vfs_getattr(struct vnode *vnode, struct vnode_attr *attr) {
    return vnode->v_ops->getattr(vnode, attr);
}

