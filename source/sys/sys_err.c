#include <sys_err.h>

static const char *const sys_err_names[] = {
    "SUCCESS",                           /* SYS_OK            =  0  */
    "INVALID_ARGUMENT",                  /* SYS_EINVAL        = -1  */
    "NO_SUCH_FILE_OR_DIRECTORY",         /* SYS_ENOENT        = -2  */
    "PERMISSION_DENIED",                 /* SYS_EACCES        = -3  */
    "BAD_FILE_DESCRIPTOR",               /* SYS_EBADF         = -4  */
    "NOT_ENOUGH_MEMORY",                 /* SYS_ENOMEM        = -5  */
    "FILE_EXISTS",                       /* SYS_EEXIST        = -6  */
    "I/O_ERROR",                         /* SYS_EIO           = -7  */
    "OPERATION_WOULD_BLOCK",             /* SYS_EAGAIN        = -8  */
    "PLATFORM_NOT_SUPPORTED",            /* SYS_ENOTSUP       = -9  */
    "PIPE_ERROR",                        /* SYS_EPIPE         = -10 */
    "NOT_CONNECTED",                     /* SYS_ENOTCONN      = -11 */
    "CONNECTION_REFUSED",                /* SYS_ECONNREFUSED  = -12 */
    "TIMED_OUT",                         /* SYS_ETIMEDOUT     = -13 */
    "CONNECTION_RESET",                  /* SYS_ECONNRESET    = -14 */
    "PROTOCOL_ERROR"                     /* SYS_EPROTO        = -15 */
};
const char *sys_err_string(int code) {
    size_t i;

    if (code >= 0)
        code = 0;
    i = (size_t)(-code);
    if (i >= sizeof(sys_err_names) / sizeof(sys_err_names[0]))
        return "kode kesalahan tidak dikenal";
    return sys_err_names[i];
}
