#ifndef PAS_SYS_RECV_H
#define PAS_SYS_RECV_H

/*
 * sys_recv.h — Kanal stream (TCP) tingkat rendah untuk menerima data.
 *
 * Implementasi: source/os/<platform>/*_recv.c
 * Di balik layar berupa soket POSIX (unix) atau WinSock (windows); bagian
 * inisialisasi WinSock (WSAStartup) hanya diperlukan dan hanya ada di
 * backend Windows.
 */

#include <stddef.h>
#include <stdint.h>
#include <sys_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sys_chan sys_chan_t;   /* opaque: fd (POSIX) / SOCKET (Windows) */

sys_err_t sys_chan_connect_tcp(const char *host, uint16_t port,
                               sys_chan_t **out);
sys_err_t sys_chan_send(sys_chan_t *c, const void *data, size_t size,
                        size_t *nsent);
/* sys_chan_recv: 0 byte terbaca berarti koneksi ditutup lawan bicara. */
sys_err_t sys_chan_recv(sys_chan_t *c, void *dst, size_t size, size_t *nrecv);
sys_err_t sys_chan_close(sys_chan_t *c);

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_RECV_H */
