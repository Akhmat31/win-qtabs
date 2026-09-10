/* unix_buffer.c — Backend POSIX: blok memori selaras-halaman (posix_memalign). */
#include <os_config.h>
#include <os_core.h>

#include <stdlib.h>
#include <string.h>

#include <sys_buf.h>
#include <sys_mem.h>

void *sys_buf_alloc_pages(size_t size)
{
    size_t align;
    void *p = NULL;

    if (size == 0)
        size = 1;
    align = sys_mem_page_size();
    if (align < sizeof(void *))
        align = sizeof(void *);

    if (posix_memalign(&p, align, size) != 0)
        return NULL;
    return p;
}

void sys_buf_free_pages(void *ptr)
{
    free(ptr);
}
