/* ============================================================================
 * src/utils/memory.h - Memory Management
 * ========================================================================== */
#ifndef FONTKIT_MEMORY_H
#define FONTKIT_MEMORY_H

#include <stddef.h>

void fk_memory_init(void);
void fk_memory_shutdown(void);

void* fk_malloc(size_t size);
void* fk_calloc(size_t count, size_t size);
void* fk_realloc(void *ptr, size_t size);
void fk_free(void *ptr);

void* fk_memcpy(void *dest, const void *src, size_t n);
void* fk_memset(void *s, int c, size_t n);

#endif /* FONTKIT_MEMORY_H */