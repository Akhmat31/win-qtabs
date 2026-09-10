/* win_recv.c --> Backend Win32: TCP (sys_chan_*)*/
#include <os_config.h>
#include <os_core.h>

#include <stdlib.h>
#include <string.h>

#include <sys_recv.h>
#include <sys_err.h>

extern sys_err_t sys_err_map(int e);

struct sys_chan {
    SOCKET s;
};

static LONG wsock_refs = 0;
static sys_err_t wsock_ensure(void)
{
    if (InterlockedIncrement(&wsock_refs) == 1) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            InterlockedDecrement(&wsock_refs);
            return SYS_ENOTSUP;
        }
    }
    return SYS_OK;
}

sys_err_t sys_chan_connect_tcp(const char *host, uint16_t port, sys_chan_t **out)
{
    struct addrinfo hints, *res = NULL, *it;
    char portstr[16];
    SOCKET s = INVALID_SOCKET;
    sys_chan_t *c;
    sys_err_t rc;
    int one = 1;

    if (!host || !out)
        return SYS_EINVAL;
    *out = NULL;

    rc = wsock_ensure();
    if (rc != SYS_OK)
        return rc;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    snprintf(portstr, sizeof portstr, "%u", (unsigned)port);

    if (getaddrinfo(host, portstr, &hints, &res) != 0)
        return SYS_EINVAL;

    for (it = res; it; it = it->ai_next) {
        s = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (s == INVALID_SOCKET)
            continue;
        if (connect(s, it->ai_addr, (int)it->ai_addrlen) == 0)
            break;
        closesocket(s);
        s = INVALID_SOCKET;
    }
    freeaddrinfo(res);
    if (s == INVALID_SOCKET)
        return sys_err_map((int)WSAGetLastError());

    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof one);
    c = (sys_chan_t *)malloc(sizeof *c);
    if (!c) {
        closesocket(s);
        return SYS_ENOMEM;
    }
    c->s = s;
    *out = c;
    return SYS_OK;
}

sys_err_t sys_chan_send(sys_chan_t *c, const void *data, size_t size, size_t *nsent)
{
    int w;

    if (!c || (size && !data))
        return SYS_EINVAL;
    if (size > 0x7FFFFFFF)
        size = 0x7FFFFFFF;
    w = send(c->s, (const char *)data, (int)size, 0);
    if (w == SOCKET_ERROR)
        return sys_err_map((int)WSAGetLastError());
    if (nsent)
        *nsent = (size_t)w;
    return SYS_OK;
}

sys_err_t sys_chan_recv(sys_chan_t *c, void *dst, size_t size, size_t *nrecv)
{
    int r;

    if (!c || (size && !dst))
        return SYS_EINVAL;
    if (size > 0x7FFFFFFF)
        size = 0x7FFFFFFF;
    r = recv(c->s, (char *)dst, (int)size, 0);
    if (r == 0) {
        if (nrecv)
            *nrecv = 0;
        return SYS_OK;
    }
    if (r == SOCKET_ERROR)
        return sys_err_map((int)WSAGetLastError());
    if (nrecv)
        *nrecv = (size_t)r;
    return SYS_OK;
}
sys_err_t sys_chan_close(sys_chan_t *c)
{
    if (!c) return SYS_EINVAL;
    closesocket(c->s);
    free(c);
    if (InterlockedDecrement(&wsock_refs) == 0) WSACleanup();
    return SYS_OK;
}