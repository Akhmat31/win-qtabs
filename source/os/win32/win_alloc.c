/*win_alloc.c --> Backend Win32: sys_mem_*. */
#include <os_config.h>
#include <os_core.h>

#include <stdlib.h>
#include <string.h>

#include <sys_mem.h>
#include <sys_err.h>

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
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (size_t)si.dwPageSize;
}
size_t sys_mem_cacheline_size(void)
{
    //// Windows does not provide a direct API to get cache line size, so we return a common value
    return 64;
}