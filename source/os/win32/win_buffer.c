/* win_buffer.c --> Backend Win32: mem-block */
#include <os_config.h>
#include <os_core.h>

#include <stdint.h>

#include <sys_buf.h>
#include <sys_mem.h>

void *sys_buf_alloc_pages(size_t size)
{
    size_t align = sys_mem_page_size();
    size_t padded;
    unsigned char *raw, *result;

    if (size == 0) size = 1;
    if (align < sizeof(void *)) align = sizeof(void *);
    /* addpadded */
    padded = size + align;
    if (padded < size) return NULL;
    raw = (unsigned char *)VirtualAlloc(NULL, padded, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!raw) return NULL;
    result = raw;
    while (((uintptr_t)result & (align - 1)) != 0)
        result++;
    if (result == raw) result += align;
    ((unsigned char **)result)[-1] = raw;
    return result;
}
void sys_buf_free_pages(void *ptr)
{
    unsigned char *raw;
    if (!ptr) return;
    raw = ((unsigned char **)ptr)[-1];
    VirtualFree(raw, 0, MEM_RELEASE);
}