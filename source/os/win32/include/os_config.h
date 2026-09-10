#ifndef PAS_OS_CONFIG_H
#define PAS_OS_CONFIG_H
#define PAS_OS_WIN32 1

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#endif /* PAS_OS_CONFIG_H */
