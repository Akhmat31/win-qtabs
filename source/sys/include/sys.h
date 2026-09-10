#ifndef PAS_SYS_H
#define PAS_SYS_H

/*
 * sys.h — Pintu masuk tunggal ke lapisan abstraksi sistem (source/sys).
 *
 * Seluruh deklarasi di sini netral-platform. Implementasi konkret disediakan
 * oleh backend di source/os/<platform>; pemilihan backend dilakukan di
 * CMakeLists.txt (bukan di dalam kode core).
 */

#include <sys_types.h>
#include <sys_err.h>
#include <sys_mem.h>
#include <sys_file.h>
#include <sys_buf.h>
#include <sys_proc.h>
#include <sys_recv.h>

#endif /* PAS_SYS_H */
