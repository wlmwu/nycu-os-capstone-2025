#ifndef CPIO_H_
#define CPIO_H_

#include "utils.h"
#include "mini_uart.h"
#include <stdint.h>


// QEMU
#define CPIO_BASE_ADDR 0x8000000
// // RPI3
// #define CPIO_BASE_ADDR 0x20000000

#define CPIO_NEWC_MAGIC "070701"
#define CPIO_END_RECORD "TRAILER!!!"
#define CPIO_PAD_LEN 4

typedef struct cpio_newc_header {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
} cpio_newc_header_t;

/**
 * @brief Retrieves information about a file from the cpio archive.
 * 
 * This function extracts the file's metadata and data from the current file header in the cpio archive.
 * It returns the next file header's address, allowing for the traversal of the archive.
 * 
 * @param hptr        A pointer to the current file header's start address in the cpio archive.
 * @param pathname    A pointer to a string where the file's path will be stored.
 * @param filesize    A pointer to an unsigned int where the file's size (in bytes) will be stored.
 * @param filedata    A pointer to a buffer where the file's data will be stored.
 * 
 * @return A pointer to the start address of the next file header in the cpio archive.
 */
cpio_newc_header_t* cpio_get_file(cpio_newc_header_t *hptr, char **pathname, char **filedata);

void cpio_ls(int argc, char **argv);
void cpio_cat(int argc, char **argv);

#endif // CPIO_H_