/* unix_alloc.c --> POSIX: sys_mem_*. */
#include <os_config.h>
#include <os_core.h>
#include <sys_mem.h>
#include <sys_err.h>

#include <stdlib.h>
#include <string.h>

void *sys_mem_alloc(size_t size)
{
    return malloc(size ? size : 1);
}
void *sys_mem_calloc(size_t count, size_t size)
{
    return calloc(count ? count : 1, size ? size : 1);
}
void *sys_mem_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size ? size : 1);
}
void sys_mem_free(void *ptr)
{
    free(ptr);
}
size_t sys_mem_page_size(void)
{
    long p = sysconf(_SC_PAGESIZE);
    return (p > 0) ? (size_t)p : 4096;
}
size_t sys_mem_cacheline_size(void)
{
#if defined(_SC_LEVEL1_DCACHE_LINESIZE)
    long n = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (n > 0)
        return (size_t)n;
#endif
    return 64;
}