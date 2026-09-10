/* sys_buf.c — Buffer byte dinamis (logika portabel).
 *
 * Bagian ini TIDAK bergantung pada platform: memori diperoleh lewat
 * sys_mem_* sehingga perilaku pertumbuhan buffer identik di semua OS.
 */

#include <sys_buf.h>
#include <sys_mem.h>

#include <string.h>
#include <stdint.h>

struct sys_buf {
    unsigned char *data;   /* NULL selama kapasitas 0 */
    size_t len;            /* jumlah byte terisi       */
    size_t cap;            /* kapasitas teralokasi     */
};

sys_err_t sys_buf_new(size_t initial_cap, sys_buf_t **out)
{
    sys_buf_t *b;

    if (!out)
        return SYS_EINVAL;
    *out = NULL;

    b = (sys_buf_t *)sys_mem_calloc(1, sizeof *b);
    if (!b)
        return SYS_ENOMEM;

    if (initial_cap > 0) {
        sys_err_t rc = sys_buf_reserve(b, initial_cap);
        if (rc != SYS_OK) {
            sys_mem_free(b);
            return rc;
        }
    }
    *out = b;
    return SYS_OK;
}

void sys_buf_free(sys_buf_t *b)
{
    if (!b)
        return;
    sys_mem_free(b->data);
    sys_mem_free(b);
}

size_t sys_buf_len(const sys_buf_t *b)
{
    return b ? b->len : 0;
}

size_t sys_buf_cap(const sys_buf_t *b)
{
    return b ? b->cap : 0;
}

const void *sys_buf_data(const sys_buf_t *b)
{
    return b ? (const void *)b->data : NULL;
}

sys_err_t sys_buf_reserve(sys_buf_t *b, size_t extra)
{
    size_t perlu, cap_baru;
    unsigned char *nd;

    if (!b)
        return SYS_EINVAL;
    if (extra <= b->cap - b->len)          /* ruang kosong sudah cukup */
        return SYS_OK;
    if (extra > SIZE_MAX - b->len)         /* cegah luapan penjumlahan */
        return SYS_ENOMEM;
    perlu = b->len + extra;

    cap_baru = b->cap ? b->cap : 256;
    while (cap_baru < perlu) {
        if (cap_baru > SIZE_MAX / 2) {
            cap_baru = perlu;
            break;
        }
        cap_baru *= 2;
    }

    nd = (unsigned char *)sys_mem_realloc(b->data, cap_baru);
    if (!nd)
        return SYS_ENOMEM;
    b->data = nd;
    b->cap = cap_baru;
    return SYS_OK;
}

sys_err_t sys_buf_append(sys_buf_t *b, const void *data, size_t len)
{
    sys_err_t rc;

    if (!b) return SYS_EINVAL;
    if (len == 0) return SYS_OK;
    if (!data) return SYS_EINVAL;

    rc = sys_buf_reserve(b, len);
    if (rc != SYS_OK) return rc;

    memcpy(b->data + b->len, data, len);
    b->len += len;
    return SYS_OK;
}
void sys_buf_clear(sys_buf_t *b)
{
    if (b) b->len = 0;
}
