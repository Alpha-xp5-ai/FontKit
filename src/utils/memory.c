/**
 * memory.c - Memory management utilities
 */

#include "memory.h"
#include <stdlib.h>
#include <string.h>

/* Simple wrappers around standard library - can be extended with pooling, tracking, etc. */

void fk_memory_init(void) {
    /* Initialize memory subsystem if needed */
}

void fk_memory_shutdown(void) {
    /* Cleanup memory subsystem if needed */
}

void* fk_malloc(size_t size) {
    return malloc(size);
}

void* fk_calloc(size_t count, size_t size) {
    return calloc(count, size);
}

void* fk_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void fk_free(void *ptr) {
    free(ptr);
}

void* fk_memcpy(void *dest, const void *src, size_t n) {
    return memcpy(dest, src, n);
}

void* fk_memset(void *s, int c, size_t n) {
    return memset(s, c, n);
}