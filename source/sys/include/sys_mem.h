#ifndef PAS_SYS_MEM_H
#define PAS_SYS_MEM_H

/*
 * sys_mem.h — Manajemen memori mentah & properti memori platform.
 * Implementasi: source/os/<platform>/*_alloc.c
 */

#include <stddef.h>
#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void  *sys_mem_alloc(size_t size);
void  *sys_mem_calloc(size_t count, size_t size);
void  *sys_mem_realloc(void *ptr, size_t size);
void   sys_mem_free(void *ptr);

/* Ukuran halaman memori & cache-line mesin (untuk tuning buffer I/O). */
size_t sys_mem_page_size(void);
size_t sys_mem_cacheline_size(void);

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_MEM_H */
