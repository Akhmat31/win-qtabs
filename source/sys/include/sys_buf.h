#ifndef PAS_SYS_BUF_H
#define PAS_SYS_BUF_H

/*
 * sys_buf.h — Buffer byte dinamis (portabel) & blok I/O selaras-halaman.
 *
 * - sys_buf_*  : logika pertumbuhan buffer murni portabel; diimplementasikan
 *                sekali di source/sys/sys_buf.c (diatas sys_mem_*).
 * - sys_buf_alloc_pages / sys_buf_free_pages : alokasi blok selaras-halaman
 *                untuk I/O langsung; implementasinya khusus-OS
 *                (source/os/<platform>/*_buffer.c).
 */

#include <stddef.h>
#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sys_buf sys_buf_t;   /* buffer byte yang bisa tumbuh */

sys_err_t   sys_buf_new(size_t initial_cap, sys_buf_t **out);
void        sys_buf_free(sys_buf_t *b);
size_t      sys_buf_len(const sys_buf_t *b);
size_t      sys_buf_cap(const sys_buf_t *b);
const void *sys_buf_data(const sys_buf_t *b);
sys_err_t   sys_buf_reserve(sys_buf_t *b, size_t extra);
sys_err_t   sys_buf_append(sys_buf_t *b, const void *data, size_t len);
void        sys_buf_clear(sys_buf_t *b);   /* isi dikosongkan, kapasitas tetap */

/* Blok memori selaras-halaman; NULL bila gagal. */
void *sys_buf_alloc_pages(size_t size);
void  sys_buf_free_pages(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_BUF_H */
