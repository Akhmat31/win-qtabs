#ifndef PAS_SYS_FILE_H
#define PAS_SYS_FILE_H

/*
 * sys_file.h — Operasi berkas tingkat rendah.
 *
 * Implementasi: source/os/<platform>/*_file.c
 * Handle berkas sengaja dibuat opaque: pemanggil tidak perlu tahu apakah di
 * balik layar berupa file descriptor (POSIX) atau HANDLE (Windows).
 */

#include <stddef.h>
#include <stdint.h>
#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sys_file sys_file_t;

enum {
    SYS_FILE_READ   = 1u << 0,   /* buka untuk baca           */
    SYS_FILE_WRITE  = 1u << 1,   /* buka untuk tulis          */
    SYS_FILE_CREATE = 1u << 2,   /* buat bila belum ada       */
    SYS_FILE_TRUNC  = 1u << 3,   /* kosongkan isi saat dibuka */
    SYS_FILE_APPEND = 1u << 4    /* tulis selalu di akhir     */
};

typedef enum sys_seek {
    SYS_SEEK_SET = 0,
    SYS_SEEK_CUR = 1,
    SYS_SEEK_END = 2
} sys_seek_t;

sys_err_t sys_file_open(const char *path, unsigned flags, sys_file_t **out);
sys_err_t sys_file_close(sys_file_t *f);
sys_err_t sys_file_read(sys_file_t *f, void *dst, size_t size, size_t *nread);
sys_err_t sys_file_write(sys_file_t *f, const void *src, size_t size, size_t *nwritten);
sys_err_t sys_file_seek(sys_file_t *f, int64_t off, sys_seek_t whence,
                        uint64_t *new_pos);
sys_err_t sys_file_size(sys_file_t *f, uint64_t *size);
sys_err_t sys_file_flush(sys_file_t *f);

int       sys_file_exists(const char *path);   /* 1 = ada, 0 = tidak ada */
sys_err_t sys_file_delete(const char *path);
sys_err_t sys_file_rename(const char *old_path, const char *new_path);

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_FILE_H */
