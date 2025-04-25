#include "fdt.h"

fdt_header_t *g_fdt_header_start;

void fdt_init(void *dtb_ptr) {
    g_fdt_header_start = dtb_ptr;
    if (bswap32(g_fdt_header_start->magic) != FDT_MAGIC) {
        uart_puts("Error: FDT header is invalid (incorrect magic number).\n");
        g_fdt_header_start = 0;
    }
}

void fdt_traverse(fdt_callback_fn_t fn) {
    if (!g_fdt_header_start) {
        uart_puts("Error: FDT not initialized before calling traverse.\n");
        return;
    }
    void *struct_ptr = (char*)g_fdt_header_start + bswap32(g_fdt_header_start->off_dt_struct);
    void *strings_ptr = (char*)g_fdt_header_start + bswap32(g_fdt_header_start->off_dt_strings);
    void *fdt_end = struct_ptr + bswap32(g_fdt_header_start->size_dt_struct);

    fdt32_t tag;
    char *name, *data;
    fdt32_t len;
    unsigned int offset;

    void *currptr = struct_ptr;
    while (currptr < fdt_end) {
        tag = bswap32(*(uint32_t*)currptr);
        name = data = (char*)NULL;
        len = 0;
        offset = FDT_TAGSIZE;

        switch (tag) {
        case FDT_BEGIN_NODE: 
            offset += ALIGN(strlen(((fdt_node_header_t*)currptr)->name) + 1, 4);       // plus 1 because of ending with '\0' 
            break;
        case FDT_END_NODE:
            break;
        case FDT_PROP:
            len = bswap32(((fdt_property_t*)currptr)->len);
            name = (char*)strings_ptr + bswap32(((fdt_property_t*)currptr)->nameoff);
            data = ((fdt_property_t*)currptr)->data;
            offset += sizeof(((fdt_property_t*)currptr)->len) + sizeof(((fdt_property_t*)currptr)->nameoff) + ALIGN(len, 4);
            break;
        case FDT_NOP:
            break;
        case FDT_END:
            break;
        default:
            uart_puts("Error: unknown FDT token. Stop.\n");
            return;
            break;
        }

        fn(tag, name, len, data);
        currptr += offset;
    }
}