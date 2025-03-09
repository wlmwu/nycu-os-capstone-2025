#ifndef CPIO_H_
#define CPIO_H_

#include "fdt.h"
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

/**
 * @brief Retrieves a file header from the cpio archive by filename.
 *
 * @param filename The name of the file to search for.
 *
 * @return A pointer to the cpio_newc_header_t structure for the found file,
 *         or NULL if the file is not found or an error occurs.
 */
cpio_newc_header_t* cpio_get_file_by_name(char *filename);

/**
 * @brief Retrieves the header of the first file in the cpio archive.
 *
 * @return A pointer to the cpio_newc_header_t structure of the first file,
 *         or NULL if an error occurs (e.g., the archive is empty or invalid).
 */
cpio_newc_header_t* cpio_get_start_file();

void cpio_initramfs_callback(fdt32_t token, char *name, fdt32_t len, char *data);
void cpio_init();


#endif // CPIO_H_