#ifndef PAS_SYS_ERR_H
#define PAS_SYS_ERR_H

/*
 * sys_err.h — Teks ramah-manusia untuk kode hasil lapisan sistem.
 * Implementasi portabel: source/sys/sys_err.c
 */

#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mengembalikan deskripsi singkat untuk kode hasil (mis. untuk log/status). */
const char *sys_err_string(int code);

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_ERR_H */
