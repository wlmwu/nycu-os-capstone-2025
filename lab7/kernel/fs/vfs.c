#include "vfs.h"
#include "fs.h"
#include "slab.h"
#include "utils.h"
#include "errno.h"
#include "tmpfs.h"

static struct mount *rootfs;

static struct filesystem tmpfs_fs = {
    .name = "tmpfs",
    .setup_mount = tmpfs_setup_mount
};

void fs_init() {
    fs_register(&tmpfs_fs);
    rootfs = kmalloc(sizeof(struct mount));
    tmpfs_fs.setup_mount(&tmpfs_fs, rootfs);
}

int vfs_lookup(const char *pathname, struct vnode **target) {
    if (strcmp(pathname, "/") == 0) {
        *target = rootfs->root;
        return 0;
    }

    struct vnode *curr = rootfs->root;
    char *path = strdup(pathname + 1);      // Skip leading '/'
    char *tok = strtok(path, "/");
    while (tok) {
        struct vnode *next;
        int retval = curr->v_ops->lookup(curr, &next, tok);
        if (retval != 0) {
            free(path);
            return retval;
        }
        curr = next;
        if (curr->mount != NULL) {
            curr = curr->mount->root;
        }
        tok = strtok(NULL, "/");
    }
    free(path);
    *target = curr;
    return 0;
}

int vfs_open(const char *pathname, int flags, struct file **target) {
    struct vnode *vn;
    int retval = vfs_lookup(pathname, &vn);
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
        if (vfs_lookup(*path ? path : "/", &parent) != 0) {
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

int vfs_mkdir(const char *pathname) {
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
    int retval = vfs_lookup(dirname, &parent);
    if (retval != 0) {
        free(path);
        return retval;
    }

    struct vnode *child;
    retval = parent->v_ops->mkdir(parent, &child, basename);
    
    free(path);
    return retval;
}

int vfs_mount(const char *target, const char *filesystem) {
    struct vnode *mount_point;
    int retval = vfs_lookup(target, &mount_point);
    if (retval != 0) return retval;

    struct filesystem *fs = fs_get_filesystem(filesystem);
    if (!fs) return -ENOENT;

    struct mount *new_mount = kmalloc(sizeof(struct mount));
    retval = fs->setup_mount(fs, new_mount);
    if (retval != 0) {
        kfree(new_mount);
        return retval;
    }

    mount_point->mount = new_mount;
    
    return 0;
}


