#include "framebuffer.h"
#include "device.h"
#include "mailbox.h"
#include "mini_uart.h"
#include "slab.h"
#include "fs.h"
#include "vfs.h"
#include "vm.h"
#include "utils.h"
#include "errno.h"
#include <stdint.h>

#define CMD_GET_INFO 0

static framebuffer_info_t fbuf_info;
uint8_t *lfb;

static void fbuf_init() {
    unsigned int __attribute__((aligned(16))) mbox[36];
    
    mbox[0] = 35 * 4;
    mbox[1] = kRCodeRequest;

    mbox[2] = kTagSetPhysicalSize; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768;  // FrameBufferInfo.height

    mbox[7] = kTagSetVirtualSize; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768;  // FrameBufferInfo.virtual_height

    mbox[12] = kTagSetVirtualOffset; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = kTagSetDepth; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = kTagSetPixelOrder; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = kTagAllocateBuffer; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = kTagGetPitch; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = kBufEndTag;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(mbox, kChArm2VC) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        framebuffer_info_t info = {
            .width = mbox[5],
            .height = mbox[6],
            .pitch = mbox[33],
            .isrgb = mbox[24]
        };
        memset(&fbuf_info, 0, sizeof(fbuf_info));
        memcpy(&fbuf_info, &info, sizeof(fbuf_info));
        lfb = (void*)PA_TO_VA((unsigned long)mbox[28]);
    } else {
        uart_printf("Error: Unable to set screen resolution to 1024x768x32\n");
    }
}

static int dev_fbuf_open(struct vnode *fvnode, struct file **target) {
    struct file *file = kmalloc(sizeof(struct file));
    file->vnode = fvnode;
    file->f_pos = 0;
    file->f_ops = fvnode->f_ops;
    file->flags = 0;                // Unused
    *target = file;
    return 0;
}

static int dev_fbuf_close(struct file *file) {
    kfree(file);
    return 0;
}

static int dev_fbuf_write(struct file *file, const void *buf, size_t count) {
    memcpy(lfb + file->f_pos, buf, count);
    file->f_pos += count;
    return count;
}

static long dev_fbuf_lseek64(struct file *file, long offset, int whence) {
    if (whence == SEEK_SET) {
        file->f_pos = offset;
        return offset;
    }
    return -EINVAL;
}

static int dev_fbuf_ioctl(struct file *file, unsigned long cmd, void *arg) {
    if (cmd == CMD_GET_INFO) {
        memcpy(arg, &fbuf_info, sizeof(framebuffer_info_t));
        return 0;
    }
    return -EINVAL;
}

static struct file_operations dev_fbuf_fops = {
    .open = dev_fbuf_open,
    .close = dev_fbuf_close,
    .write = dev_fbuf_write,
    .lseek64 = dev_fbuf_lseek64,
    .ioctl = dev_fbuf_ioctl,
};

void dev_fbuf_init() {
    fbuf_init();
    dev_register(DEV_FRAMEBUFFER_DEV, &dev_fbuf_fops);
    vfs_mknod(fs_get_root()->root, "/dev/framebuffer", DEV_FRAMEBUFFER_DEV);
}