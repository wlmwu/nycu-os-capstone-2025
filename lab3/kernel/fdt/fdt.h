#ifndef FDT_H_
#define FDT_H_

#include "mini_uart.h"
#include "utils.h"
#include <stdint.h>
#include <stddef.h>

// https://github.com/torvalds/linux/blob/master/scripts/dtc/libfdt/libfdt_env.h

typedef uint32_t fdt32_t;
typedef uint64_t fdt64_t;

// https://github.com/torvalds/linux/blob/master/scripts/dtc/libfdt/fdt.h

typedef struct fdt_header {
	fdt32_t magic;			 /* magic word FDT_MAGIC */
	fdt32_t totalsize;		 /* total size of DT block */
	fdt32_t off_dt_struct;		 /* offset to structure */
	fdt32_t off_dt_strings;		 /* offset to strings */
	fdt32_t off_mem_rsvmap;		 /* offset to memory reserve map */
	fdt32_t version;		 /* format version */
	fdt32_t last_comp_version;	 /* last compatible version */

	/* version 2 fields below */
	fdt32_t boot_cpuid_phys;	 /* Which physical CPU id we're
					    booting on */
	/* version 3 fields below */
	fdt32_t size_dt_strings;	 /* size of the strings block */

	/* version 17 fields below */
	fdt32_t size_dt_struct;		 /* size of the structure block */
} fdt_header_t;

struct fdt_reserve_entry {
	fdt64_t address;
	fdt64_t size;
};

typedef struct fdt_node_header {
	fdt32_t tag;
	char name[];
} fdt_node_header_t;

typedef struct fdt_property {
	fdt32_t tag;
	fdt32_t len;
	fdt32_t nameoff;
	char data[];
} fdt_property_t;

#define FDT_MAGIC	0xd00dfeed	/* 4: version, 4: total size */
#define FDT_TAGSIZE	sizeof(fdt32_t)

#define FDT_BEGIN_NODE	0x1		/* Start node: full name */
#define FDT_END_NODE	0x2		/* End node */
#define FDT_PROP	0x3		/* Property: name off,
					   size, content */
#define FDT_NOP		0x4		/* nop */
#define FDT_END		0x9

#define FDT_V1_SIZE	(7*sizeof(fdt32_t))
#define FDT_V2_SIZE	(FDT_V1_SIZE + sizeof(fdt32_t))
#define FDT_V3_SIZE	(FDT_V2_SIZE + sizeof(fdt32_t))
#define FDT_V16_SIZE	FDT_V3_SIZE
#define FDT_V17_SIZE	(FDT_V16_SIZE + sizeof(fdt32_t))


/**
 * @brief A function pointer type for callbacks that process device tree properties.
 * 
 * @param token   A 32-bit token (aka tag) representing a unique identifier for the property.
 * @param name    A pointer to the name of the property (string).
 * @param len     A 32-bit integer representing the length of the property data.
 * @param data    A pointer to the raw property data.
 */
typedef void (*fdt_callback_fn_t)(fdt32_t token, char *name, fdt32_t len, char *data);

void fdt_init(void *dtb_ptr);
void fdt_traverse(fdt_callback_fn_t fn);

#endif // FDT_H_
