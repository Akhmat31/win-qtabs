/* unix_recv.c — Backend POSIX: kanal TCP (sys_chan_*).
 *
 * Tanpa dependensi Qt: murni soket POSIX. Pemetaan hostname→alamat
 * dilakukan dengan getaddrinfo sehingga mendukung IPv4/IPv6.
 */

#include <os_config.h>
#include <os_core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <sys_recv.h>
#include <sys_err.h>

extern sys_err_t sys_err_map(int e);

struct sys_chan {
    int fd;
};

sys_err_t sys_chan_connect_tcp(const char *host, uint16_t port, sys_chan_t **out)
{
    struct addrinfo hints, *res = NULL, *it;
    char portstr[16];
    int fd = -1, rc, one = 1;

    if (!host || !out)
        return SYS_EINVAL;
    *out = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    snprintf(portstr, sizeof portstr, "%u", (unsigned)port);

    rc = getaddrinfo(host, portstr, &hints, &res);
    if (rc != 0)
        return SYS_EINVAL;   /* nama tidak dikenal → EINVAL */

    for (it = res; it; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0)
            continue;
        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0)
            break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0)
        return sys_err_map(errno);

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    {
        sys_chan_t *c = (sys_chan_t *)malloc(sizeof *c);
        if (!c) {
            close(fd);
            return SYS_ENOMEM;
        }
        c->fd = fd;
        *out = c;
    }
    return SYS_OK;
}

sys_err_t sys_chan_send(sys_chan_t *c, const void *data, size_t size,
                        size_t *nsent)
{
    ssize_t w;

    if (!c || (size && !data))
        return SYS_EINVAL;
    w = send(c->fd, data, size, MSG_NOSIGNAL);
    if (w < 0)
        return sys_err_map(errno);
    if (nsent)
        *nsent = (size_t)w;
    return SYS_OK;
}

sys_err_t sys_chan_recv(sys_chan_t *c, void *dst, size_t size, size_t *nrecv)
{
    ssize_t r;

    if (!c || (size && !dst))
        return SYS_EINVAL;
    r = recv(c->fd, dst, size, 0);
    if (r == 0) {
        if (nrecv)
            *nrecv = 0;      /* EOF: lawan bicara menutup koneksi */
        return SYS_OK;
    }
    if (r < 0)
        return sys_err_map(errno);
    if (nrecv)
        *nrecv = (size_t)r;
    return SYS_OK;
}

sys_err_t sys_chan_close(sys_chan_t *c)
{
    int rc;

    if (!c)
        return SYS_EINVAL;
    rc = close(c->fd);
    free(c);
    return (rc == 0) ? SYS_OK : sys_err_map(errno);
}
