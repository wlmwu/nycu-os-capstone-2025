#ifndef SLAB_H_
#define SLAB_H_

#include <stddef.h>

void slab_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif // SLAB_H_