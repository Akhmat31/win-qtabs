/* win_file.c --> Backend Win32: sys_file_*.*/
#include <os_config.h>
#include <os_core.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys_file.h>
#include <sys_err.h>

struct sys_file {
    HANDLE h;
};
sys_err_t sys_err_map(int e)
{
    switch ((DWORD)e) {
    case ERROR_SUCCESS:            return SYS_OK;
    case ERROR_INVALID_PARAMETER:
    case ERROR_BAD_ARGUMENTS:      return SYS_EINVAL;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:     return SYS_ENOENT;
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:  return SYS_EACCES;
    case ERROR_INVALID_HANDLE:     return SYS_EBADF;
    case ERROR_OUTOFMEMORY:
    case ERROR_NOT_ENOUGH_MEMORY:  return SYS_ENOMEM;
    case ERROR_ALREADY_EXISTS:     return SYS_EEXIST;
    case ERROR_DISK_FULL:
    case ERROR_WRITE_FAULT:
    case ERROR_READ_FAULT:         return SYS_EIO;
    case ERROR_IO_PENDING:         return SYS_EAGAIN;
    case ERROR_NOT_SUPPORTED:      return SYS_ENOTSUP;
    case ERROR_BROKEN_PIPE:        return SYS_EPIPE;
    case ERROR_CONNECTION_REFUSED: return SYS_ECONNREFUSED;
    case ERROR_TIMEOUT:            return SYS_ETIMEDOUT;
    case ERROR_CONNECTION_ABORTED: return SYS_ECONNRESET;
    default:                       return SYS_EIO;
    }
}

/* Bit-flags netral-platform --> dwDesiredAccess/dwCreationDisposition Win32. */
static void flags_to_win(unsigned f, DWORD *access, DWORD *creation)
{
    if ((f & SYS_FILE_READ) && (f & SYS_FILE_WRITE))
        *access = GENERIC_READ | GENERIC_WRITE;
    else if (f & SYS_FILE_WRITE)
        *access = GENERIC_WRITE;
    else
        *access = GENERIC_READ;
    if (f & SYS_FILE_CREATE) {
        if (f & SYS_FILE_TRUNC)
            *creation = CREATE_ALWAYS;
        else
            *creation = OPEN_ALWAYS;
    } else if (f & SYS_FILE_TRUNC) {
        *creation = TRUNCATE_EXISTING;
    } else {
        *creation = OPEN_EXISTING;
    }
}
sys_err_t sys_file_open(const char *path, unsigned flags, sys_file_t **out)
{
    sys_file_t *f;
    DWORD access, creation, share;
    HANDLE h;

    if (!path || !out)
        return SYS_EINVAL;
    *out = NULL;

    flags_to_win(flags, &access, &creation);
    share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    h = CreateFileA(path, access, share, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return sys_err_map((int)GetLastError());
    f = (sys_file_t *)malloc(sizeof *f);
    if (!f) {
        CloseHandle(h);
        return SYS_ENOMEM;
    }
    f->h = h;
    *out = f;
    return SYS_OK;
}
sys_err_t sys_file_close(sys_file_t *f)
{
    BOOL ok;

    if (!f)
        return SYS_EINVAL;
    ok = CloseHandle(f->h);
    free(f);
    return ok ? SYS_OK : sys_err_map((int)GetLastError());
}

sys_err_t sys_file_read(sys_file_t *f, void *dst, size_t size, size_t *nread)
{
    DWORD r = 0;

    if (!f || (size && !dst))
        return SYS_EINVAL;
    if (size > 0xFFFFFFFFu)             
        size = 0xFFFFFFFFu;
    if (!ReadFile(f->h, dst, (DWORD)size, &r, NULL))
        return sys_err_map((int)GetLastError());
    if (nread)
        *nread = (size_t)r;
    return SYS_OK;
}

sys_err_t sys_file_write(sys_file_t *f, const void *src, size_t size, size_t *nwritten)
{
    DWORD w = 0;

    if (!f || (size && !src))
        return SYS_EINVAL;
    if (size > 0xFFFFFFFFu)
        size = 0xFFFFFFFFu;
    if (!WriteFile(f->h, src, (DWORD)size, &w, NULL))
        return sys_err_map((int)GetLastError());
    if (nwritten)
        *nwritten = (size_t)w;
    return SYS_OK;
}

sys_err_t sys_file_seek(sys_file_t *f, int64_t off, sys_seek_t whence,
                        uint64_t *new_pos)
{
    LARGE_INTEGER li, out;
    DWORD method;

    if (!f)
        return SYS_EINVAL;
    switch (whence) {
    case SYS_SEEK_SET: method = FILE_BEGIN;   break;
    case SYS_SEEK_CUR: method = FILE_CURRENT; break;
    case SYS_SEEK_END: method = FILE_END;     break;
    default:           return SYS_EINVAL;
    }
    li.QuadPart = (LONGLONG)off;
    if (!SetFilePointerEx(f->h, li, &out, method))
        return sys_err_map((int)GetLastError());
    if (new_pos)
        *new_pos = (uint64_t)out.QuadPart;
    return SYS_OK;
}

sys_err_t sys_file_size(sys_file_t *f, uint64_t *size)
{
    LARGE_INTEGER sz;

    if (!f || !size)
        return SYS_EINVAL;
    if (!GetFileSizeEx(f->h, &sz))
        return sys_err_map((int)GetLastError());
    *size = (uint64_t)sz.QuadPart;
    return SYS_OK;
}

sys_err_t sys_file_flush(sys_file_t *f)
{
    if (!f)
        return SYS_EINVAL;
    if (!FlushFileBuffers(f->h))
        return sys_err_map((int)GetLastError());
    return SYS_OK;
}

int sys_file_exists(const char *path)
{
    DWORD a;
    if (!path)
        return 0;
    a = GetFileAttributesA(path);
    return (a != INVALID_FILE_ATTRIBUTES) ? 1 : 0;
}

sys_err_t sys_file_delete(const char *path)
{
    if (!path)
        return SYS_EINVAL;
    if (!DeleteFileA(path))
        return sys_err_map((int)GetLastError());
    return SYS_OK;
}

sys_err_t sys_file_rename(const char *old_path, const char *new_path)
{
    if (!old_path || !new_path)
        return SYS_EINVAL;
    if (!MoveFileA(old_path, new_path))
        return sys_err_map((int)GetLastError());
    return SYS_OK;
}