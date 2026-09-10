/* unix_process.c — Backend POSIX: sys_proc_*.
 *
 * Siklus hidup handle pada backend ini:
 *   sys_proc_start → fork/exec (handle valid)
 *   sys_proc_wait  → jika proses selesai, p di-reap (waitpid) dan di-close;
 *                    nilai *exit_code diambil dari status waitpid.
 *   sys_proc_close  → hanya membebaskan memori wrapper, tidak men-sinyali.
 */

#include <os_config.h>
#include <os_core.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include <sys_proc.h>
#include <sys_err.h>

extern sys_err_t sys_err_map(int e);   /* dipetakan di unix_file.c (backend sama) */

struct sys_proc {
    pid_t pid;
    int   reaped;       /* 1 = sudah di-reap (dikumpulkan statusnya) */
    int   exit_code;
};

sys_err_t sys_proc_start(const char *program, const char *const args[],
                         sys_proc_t **out)
{
    sys_proc_t *p;
    pid_t pid;
    size_t n = 0;
    char **argv;

    if (!program || !out)
        return SYS_EINVAL;
    *out = NULL;

    /* Hitung jumlah argumen untuk menyusun argv[]. */
    if (args) {
        while (args[n])
            n++;
    }
    argv = (char **)calloc(n + 2, sizeof(char *));
    if (!argv)
        return SYS_ENOMEM;
    argv[0] = (char *)program;
    for (size_t i = 0; i < n; i++)
        argv[i + 1] = (char *)args[i];
    argv[n + 1] = NULL;

    p = (sys_proc_t *)calloc(1, sizeof *p);
    if (!p) {
        free(argv);
        return SYS_ENOMEM;
    }

    pid = fork();
    if (pid < 0) {
        free(argv);
        free(p);
        return sys_err_map(errno);
    }
    if (pid == 0) {
        /* Anak: tidak boleh memakai malloc di jalur exec — langsung execvp. */
        execvp(program, argv);
        _exit(127);
    }

    free(argv);
    p->pid = pid;
    p->reaped = 0;
    p->exit_code = -1;
    *out = p;
    return SYS_OK;
}

sys_err_t sys_proc_wait(sys_proc_t *p, int timeout_ms, int *exit_code,
                        int *running)
{
    int status = 0;
    pid_t r;

    if (!p || !running)
        return SYS_EINVAL;
    *running = 1;
    if (exit_code)
        *exit_code = 0;

    if (p->reaped) {
        *running = 0;
        if (exit_code)
            *exit_code = p->exit_code;
        return SYS_OK;
    }

    if (timeout_ms == 0) {
        /* Non-blok: WNOHANG. */
        r = waitpid(p->pid, &status, WNOHANG);
        if (r == 0)
            return SYS_OK;              /* masih berjalan */
        if (r < 0)
            return sys_err_map(errno);
    } else {
        /* Blok sampai selesai (backoff pendek agar tetap responsif). */
        for (;;) {
            r = waitpid(p->pid, &status, WNOHANG);
            if (r == p->pid)
                break;
            if (r < 0)
                return sys_err_map(errno);
            if (timeout_ms > 0) {
                if (timeout_ms <= 10)
                    return SYS_OK;      /* batas waktu habis, masih berjalan */
                timeout_ms -= 10;
            }
            usleep(10000);              /* 10 ms */
        }
    }

    p->reaped = 1;
    p->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    *running = 0;
    if (exit_code)
        *exit_code = p->exit_code;
    return SYS_OK;
}

sys_err_t sys_proc_kill(sys_proc_t *p)
{
    if (!p)
        return SYS_EINVAL;
    if (p->reaped)
        return SYS_OK;
    if (kill(p->pid, SIGTERM) != 0)
        return sys_err_map(errno);
    return SYS_OK;
}

sys_err_t sys_proc_close(sys_proc_t *p)
{
    if (!p)
        return SYS_EINVAL;
    if (!p->reaped)
        waitpid(p->pid, NULL, WNOHANG); /* reap kalau memang sudah selesai */
    free(p);
    return SYS_OK;
}

uint64_t sys_proc_self_id(void)
{
    return (uint64_t)getpid();
}
