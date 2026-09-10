#ifndef PAS_SYS_PROC_H
#define PAS_SYS_PROC_H
/*
 * sys_proc.h — Proses: memulai / menunggu / menghentikan proses lain.
 * Implementasi: source/os/<platform>/*_process.c
 */
#include <stdint.h>
#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sys_proc sys_proc_t;   /* opaque: pid (POSIX) / handle (Windows) */
/* args: daftar argumen berakhir NULL (boleh NULL = tanpa argumen).
 * args[0] biasanya nama program, tetapi tidak wajib. */
sys_err_t sys_proc_start(const char *program, const char *const args[], sys_proc_t **out);
/* timeout_ms: <0 = tunggu sampai selesai; 0 = non-blok; >0 = batas waktu.
 * Saat kembali: *running = 0 → selesai dan *exit_code valid;
 *               *running = 1 → masih berjalan. */
sys_err_t sys_proc_wait(sys_proc_t *p, int timeout_ms, int *exit_code, int *running);
sys_err_t sys_proc_kill(sys_proc_t *p);   /* hentikan proses (SIGTERM/TerminateProcess) */
sys_err_t sys_proc_close(sys_proc_t *p);  /* bebaskan handle; proses TIDAK ikut dihentikan */
uint64_t  sys_proc_self_id(void);         /* pid proses yang sedang berjalan */

#ifdef __cplusplus
}
#endif
#endif /* PAS_SYS_PROC_H */