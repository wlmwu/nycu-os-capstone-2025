#include "cpio.h"
#include "utils.h"
#include "mini_uart.h"
#include "mm.h"
#include "mmu.h"
#include <stddef.h>

static void *cpio_start;
static void *cpio_end;

cpio_newc_header_t* cpio_get_file(cpio_newc_header_t *hptr, char **pathname, unsigned int *filesize, char **filedata) {
    if (!hptr || strncmp(hptr->c_magic, CPIO_NEWC_MAGIC, sizeof(hptr->c_magic)) != 0) {
        return NULL;
    }
    unsigned int fsize;
    if (!filesize) filesize = &fsize;

    char *pfdata;
    if (!filedata) filedata = &pfdata;

    char *ppname;
    if (!pathname) pathname = &ppname;

    unsigned int pathname_size = hexstr2uint(hptr->c_namesize);
    *filesize = hexstr2uint(hptr->c_filesize);

    *pathname = (char*)hptr + sizeof(cpio_newc_header_t);
    if (strcmp(*pathname, CPIO_END_RECORD) == 0) {
        *pathname = NULL;
    }
    unsigned int offset = (unsigned int)((pathname_size + sizeof(cpio_newc_header_t) + CPIO_PAD_LEN - 1) / CPIO_PAD_LEN) * CPIO_PAD_LEN;
    *filedata = ((*filesize) != 0) ? (char*)hptr + offset : 0;
    
    offset += (unsigned int)(((*filesize) + CPIO_PAD_LEN - 1) / CPIO_PAD_LEN) * CPIO_PAD_LEN;
    cpio_newc_header_t *next_hptr = (cpio_newc_header_t*)((char*)hptr + offset);

    return next_hptr;
}

cpio_newc_header_t* cpio_get_file_by_name(char *filename) {
    char *pathname;
    cpio_newc_header_t *hptr = (cpio_newc_header_t*)cpio_start;
    do {
        cpio_newc_header_t *nhptr = cpio_get_file(hptr, &pathname, NULL, NULL);
        if (strcmp(pathname, filename) == 0 ||
           (strncmp(pathname, "./", sizeof("./") - 1) == 0 && strcmp(pathname + (sizeof("./") - 1), filename) == 0)) {      // sizeof("./") is length of "./\0"
            return hptr;
        } else if (strcmp(pathname, CPIO_END_RECORD) == 0) {
            return NULL;
        } 
        hptr = nhptr;
    } while (hptr);

    return NULL;
}

cpio_newc_header_t* cpio_get_start_file() {
    char *pathname;
    cpio_newc_header_t *hptr = (cpio_newc_header_t*)cpio_start;
    cpio_get_file(hptr, &pathname, NULL, NULL);
    if (!pathname) {
        return NULL;
    } else {
        return hptr;
    }
}

void cpio_initramfs_callback(fdt32_t token, char *name, fdt32_t len, char *data) {
    if (token == FDT_PROP && strcmp(name, "linux,initrd-start") == 0) {
        cpio_start = (void*)PA_TO_VA(bswap32(*(uint32_t*)data));
    }
    if (token == FDT_PROP && strcmp(name, "linux,initrd-end") == 0) {
        cpio_end = (void*)PA_TO_VA(bswap32(*(uint32_t*)data));
    }
}

void cpio_init() {
    fdt_traverse(cpio_initramfs_callback);
    if (!cpio_start) {
        uart_puts("Error: CPIO initialization failed.\n");
    }
    if (cpio_start && cpio_end) {
        mm_reserve(VA_TO_PA((uintptr_t)cpio_start), VA_TO_PA((uintptr_t)cpio_end));
    }
}