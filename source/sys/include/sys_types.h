#ifndef PAS_SYS_TYPES_H
#define PAS_SYS_TYPES_H

/*
 * sys_types.h — Tipe dasar & kode hasil lapisan sistem (netral-platform).
 *
 * Kode hasil (sys_err_t) sengaja BUKAN errno mentah agar nilainya konsisten
 * di semua sistem operasi; masing-masing backend di source/os bertugas
 * memetakan error asli OS (errno / GetLastError / WSAGetLastError) ke tabel
 * di bawah ini.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int sys_err_t;              /* 0 = sukses, negatif = kode kesalahan */

#define SYS_OK              0
#define SYS_EINVAL          (-1)    /* argumen tidak valid                */
#define SYS_ENOENT          (-2)    /* berkas / entri tidak ditemukan     */
#define SYS_EACCES          (-3)    /* akses ditolak                      */
#define SYS_EBADF           (-4)    /* deskriptor / handle tidak valid    */
#define SYS_ENOMEM          (-5)    /* memori tidak cukup                 */
#define SYS_EEXIST          (-6)    /* entri sudah ada                    */
#define SYS_EIO             (-7)    /* kesalahan I/O umum                 */
#define SYS_EAGAIN          (-8)    /* operasi belum bisa diselesaikan    */
#define SYS_EWOULDBLOCK     SYS_EAGAIN
#define SYS_ENOTSUP         (-9)    /* tidak didukung backend ini         */
#define SYS_EPIPE           (-10)   /* saluran ditutup lawan bicara       */
#define SYS_ENOTCONN        (-11)   /* koneksi belum tersambung           */
#define SYS_ECONNREFUSED    (-12)   /* koneksi ditolak                    */
#define SYS_ETIMEDOUT       (-13)   /* waktu tunggu habis                 */
#define SYS_ECONNRESET      (-14)   /* koneksi di-reset lawan bicara      */
#define SYS_EPROTO          (-15)   /* kesalahan protokol jaringan        */

#ifdef __cplusplus
}
#endif

#endif /* PAS_SYS_TYPES_H */
