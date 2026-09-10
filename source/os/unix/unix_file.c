/* unix_file.c — Backend POSIX: sys_file_*.
 *
 * sys_file_t hanyalah wrapper tipis di atas int fd. Perhatikan bahwa
 * seluruh file ini TIDAK menyertakan header Qt/Windows apa pun; satusatunya
 * "kontrak" dengan pemanggil adalah API di source/sys/include/sys_file.h.
 */

#include <os_config.h>
#include <os_core.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <sys_file.h>
#include <sys_err.h>

struct sys_file {
    int fd;
};

/* Peta errno POSIX → kode hasil netral-platform. */
sys_err_t sys_err_map(int e)
{
    switch (e) {
    case EINVAL:      return SYS_EINVAL;
    case ENOENT:      return SYS_ENOENT;
    case EACCES:
    case EPERM:       return SYS_EACCES;
    case EBADF:       return SYS_EBADF;
    case ENOMEM:      return SYS_ENOMEM;
    case EEXIST:      return SYS_EEXIST;
    case EIO:         return SYS_EIO;
    case EAGAIN:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
                      return SYS_EAGAIN;
    case ENOTSUP:
#if defined(EOPNOTSUPP) && EOPNOTSUPP != ENOTSUP
    case EOPNOTSUPP:
#endif
                      return SYS_ENOTSUP;
    case EPIPE:       return SYS_EPIPE;
    case ENOTCONN:    return SYS_ENOTCONN;
    case ECONNREFUSED:return SYS_ECONNREFUSED;
    case ETIMEDOUT:   return SYS_ETIMEDOUT;
    case ECONNRESET:  return SYS_ECONNRESET;
    default:          return SYS_EIO;
    }
}

/* Terjemahan bit-flags netral-platform → flag POSIX open(). */
static int flags_to_posix(unsigned f)
{
    int oflag = 0;

    if ((f & SYS_FILE_READ) && (f & SYS_FILE_WRITE))
        oflag = O_RDWR;
    else if (f & SYS_FILE_WRITE)
        oflag = O_WRONLY;
    else
        oflag = O_RDONLY;

    if (f & SYS_FILE_CREATE) oflag |= O_CREAT;
    if (f & SYS_FILE_TRUNC)  oflag |= O_TRUNC;
    if (f & SYS_FILE_APPEND) oflag |= O_APPEND;
    return oflag;
}

sys_err_t sys_file_open(const char *path, unsigned flags, sys_file_t **out)
{
    sys_file_t *f;
    int fd, oflag;

    if (!path || !out)
        return SYS_EINVAL;
    *out = NULL;

    oflag = flags_to_posix(flags);
    fd = open(path, oflag, 0666);
    if (fd < 0)
        return sys_err_map(errno);

    f = (sys_file_t *)malloc(sizeof *f);
    if (!f) {
        close(fd);
        return SYS_ENOMEM;
    }
    f->fd = fd;
    *out = f;
    return SYS_OK;
}

sys_err_t sys_file_close(sys_file_t *f)
{
    int rc;

    if (!f)
        return SYS_EINVAL;
    rc = close(f->fd);
    free(f);
    return (rc == 0) ? SYS_OK : sys_err_map(errno);
}

sys_err_t sys_file_read(sys_file_t *f, void *dst, size_t size, size_t *nread)
{
    ssize_t r;

    if (!f || (size && !dst))
        return SYS_EINVAL;
    r = read(f->fd, dst, size);
    if (r < 0)
        return sys_err_map(errno);
    if (nread)
        *nread = (size_t)r;
    return SYS_OK;
}

sys_err_t sys_file_write(sys_file_t *f, const void *src, size_t size,
                         size_t *nwritten)
{
    ssize_t w;

    if (!f || (size && !src))
        return SYS_EINVAL;
    w = write(f->fd, src, size);
    if (w < 0)
        return sys_err_map(errno);
    if (nwritten)
        *nwritten = (size_t)w;
    return SYS_OK;
}

sys_err_t sys_file_seek(sys_file_t *f, int64_t off, sys_seek_t whence,
                        uint64_t *new_pos)
{
    int w;
    off_t r;

    if (!f)
        return SYS_EINVAL;
    switch (whence) {
    case SYS_SEEK_SET: w = SEEK_SET; break;
    case SYS_SEEK_CUR: w = SEEK_CUR; break;
    case SYS_SEEK_END: w = SEEK_END; break;
    default:           return SYS_EINVAL;
    }
    r = lseek(f->fd, (off_t)off, w);
    if (r < 0)
        return sys_err_map(errno);
    if (new_pos)
        *new_pos = (uint64_t)r;
    return SYS_OK;
}

sys_err_t sys_file_size(sys_file_t *f, uint64_t *size)
{
    struct stat st;

    if (!f || !size)
        return SYS_EINVAL;
    if (fstat(f->fd, &st) != 0)
        return sys_err_map(errno);
    *size = (uint64_t)st.st_size;
    return SYS_OK;
}

sys_err_t sys_file_flush(sys_file_t *f)
{
    if (!f)
        return SYS_EINVAL;
    if (fsync(f->fd) != 0)
        return sys_err_map(errno);
    return SYS_OK;
}

int sys_file_exists(const char *path)
{
    struct stat st;
    if (!path)
        return 0;
    return (stat(path, &st) == 0) ? 1 : 0;
}

sys_err_t sys_file_delete(const char *path)
{
    if (!path)
        return SYS_EINVAL;
    if (unlink(path) != 0)
        return sys_err_map(errno);
    return SYS_OK;
}

sys_err_t sys_file_rename(const char *old_path, const char *new_path)
{
    if (!old_path || !new_path)
        return SYS_EINVAL;
    if (rename(old_path, new_path) != 0)
        return sys_err_map(errno);
    return SYS_OK;
}
