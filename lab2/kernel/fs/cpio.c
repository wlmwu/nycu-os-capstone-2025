#include "cpio.h"

cpio_newc_header_t* cpio_get_file(cpio_newc_header_t *hptr, char **pathname, char **filedata) {
    if (strncmp(hptr->c_magic, CPIO_NEWC_MAGIC, sizeof(hptr->c_magic)) != 0) {
        return 0;
    }
    unsigned int pathname_size = hexstr2uint(hptr->c_namesize);
    unsigned int filesize = hexstr2uint(hptr->c_filesize);

    *pathname = (char*)hptr + sizeof(cpio_newc_header_t);
    unsigned int offset = (unsigned int)((pathname_size + sizeof(cpio_newc_header_t) + CPIO_PAD_LEN - 1) / CPIO_PAD_LEN) * CPIO_PAD_LEN;
    *filedata = (filesize != 0) ? (char*)hptr + offset : 0;
    
    offset += (unsigned int)((filesize + CPIO_PAD_LEN - 1) / CPIO_PAD_LEN) * CPIO_PAD_LEN;
    cpio_newc_header_t *next_hptr = (cpio_newc_header_t*)((char*)hptr + offset);

    return next_hptr;
}

void cpio_ls(int argc, char **argv) {
    char *pathname;
    char *filedata;
    cpio_newc_header_t *hptr = (cpio_newc_header_t*)CPIO_BASE_ADDR;
    do {
        hptr = cpio_get_file(hptr, &pathname, &filedata);
        if (strcmp(pathname, CPIO_END_RECORD) == 0) {
            break;
        }
        uart_puts(pathname);
        uart_puts("\n");
    } while (hptr);
}

void cpio_cat(int argc, char **argv) {
    if (argc <= 1) {
        uart_puts(" : Is a directory\n");
        return;
    }
    char *target_filename = argv[1];
    
    char *pathname;
    unsigned int filesize;
    char *filedata;
    cpio_newc_header_t *hptr = (cpio_newc_header_t*)CPIO_BASE_ADDR;
    do {
        hptr = cpio_get_file(hptr, &pathname, &filedata);
        if (strcmp(pathname, target_filename) == 0 ||
           (strncmp(pathname, "./", sizeof("./") - 1) == 0 && strcmp(pathname + (sizeof("./") - 1), target_filename) == 0)) {      // sizeof("./") is length of "./\0"
            if (filedata) {
                uart_puts(filedata);
                uart_puts("\n");
            }
            break;
        } else if (strcmp(pathname, CPIO_END_RECORD) == 0) {
            uart_puts(target_filename);
            uart_puts(": No such file or directory\n");
            break;
        } 
    } while (hptr);
}