/* win_process.c --> Backend Win32: sys_proc_*.*/
#include <os_config.h>
#include <os_core.h>

#include <stdlib.h>
#include <string.h>

#include <sys_proc.h>
#include <sys_err.h>

extern sys_err_t sys_err_map(int e);   /* defined in= win_file.c */

struct sys_proc {
    HANDLE proc;
    int    exited;
    int    exit_code;
};

sys_err_t sys_proc_start(const char *program, const char *const args[], sys_proc_t **out)
{
    sys_proc_t *p;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    size_t total = 1;/*for handler '\0' null terminator */
    const char *const *a;
    char *cmdline, *q;
    BOOL ok;

    if (!program || !out)
        return SYS_EINVAL;
    *out = NULL;

    if (args) {
        for (a = args; *a; a++)
            total += strlen(*a) + 1;
    }
    cmdline = (char *)malloc(total);
    if (!cmdline)
        return SYS_ENOMEM;

    q = cmdline;
    *q++ = '"';
    for (const char *s = program; *s; s++)
        *q++ = *s;
    *q++ = '"';
    if (args) {
        for (a = args; *a; a++) {
            *q++ = ' ';
            for (const char *s = *a; *s; s++)
                *q++ = *s;
        }
    }
    *q = '\0';
    p = (sys_proc_t *)calloc(1, sizeof *p);
    if (!p) {
        free(cmdline);
        return SYS_ENOMEM;
    }
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    memset(&pi, 0, sizeof pi);

    ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmdline);
    if (!ok) {
        free(p);
        return sys_err_map((int)GetLastError());
    }

    CloseHandle(pi.hThread);         
    p->proc = pi.hProcess;
    p->exited = 0;
    p->exit_code = 0;
    *out = p;
    return SYS_OK;
}
sys_err_t sys_proc_wait(sys_proc_t *p, int timeout_ms, int *exit_code, int *running)
{
    DWORD wait_ms, rc;
    DWORD code;

    if (!p || !running)
        return SYS_EINVAL;
    *running = 1;
    if (exit_code) *exit_code = 0;
    if (p->exited) {
        *running = 0;
        if (exit_code) *exit_code = p->exit_code;
        return SYS_OK;
    }
    wait_ms = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    rc = WaitForSingleObject(p->proc, wait_ms);
    if (rc == WAIT_TIMEOUT) return SYS_OK;
    if (GetExitCodeProcess(p->proc, &code)) {
        p->exited = 1;
        p->exit_code = (int)code;
    }
    *running = 0;
    if (exit_code) *exit_code = p->exit_code;
    return (rc == WAIT_OBJECT_0) ? SYS_OK : sys_err_map((int)GetLastError());
}
sys_err_t sys_proc_kill(sys_proc_t *p)
{
    if (!p) return SYS_EINVAL;
    if (p->exited) return SYS_OK;
    if (!TerminateProcess(p->proc, 1)) return sys_err_map((int)GetLastError());
    return SYS_OK;
}

sys_err_t sys_proc_close(sys_proc_t *p)
{
    if (!p) return SYS_EINVAL;
    CloseHandle(p->proc);
    free(p);
    return SYS_OK;
}
uint64_t sys_proc_self_id(void)
{
    return (uint64_t)GetCurrentProcessId();
}