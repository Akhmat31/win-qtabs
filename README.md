**desktop window built with Qt, SDL2, and a portable system abstraction layer**

---

The project pairs a Qt Widgets front end with a **portable, OS-neutral system abstraction layer** (`source/sys`)
that is backed by a single per-platform implementation (`source/os/<platform>`),
chosen automatically at configure time by CMake.

The project is intended as a base/foundation: the window shell, the build system,
and the platform abstraction are all in place so that real application features
(buffers, files, processes, TCP channels, memory management) can be layered on top
without ever writing platform-specific code in the application core.

---

## Technical Information

### Stack

| Component    | Detail                                             |
|--------------|----------------------------------------------------|
| Language     | C++17 (front end), C99 (system layer)              |
| GUI          | Qt Widgets (Qt6 preferred, Qt5 fallback)           |
| Multimedia   | SDL2 (linked; reserved for future use)             |
| Build system | CMake ≥ 3.16                                       |
| Targets      | `base_window` (executable), `pas_sys` (static lib) |

### Dependencies

- **Qt6** (Widgets) — falls back to **Qt5** automatically if Qt6 is not found.
- **SDL2** — required; uses the imported target `SDL2::SDL2` when available.
- **ws2_32** (Windows only) — linked for WinSock support in the `sys_chan_*` API.


### System Abstraction Layer (`source/sys`)

All headers live in `source/sys/include` and are included with angle brackets
(`#include <sys.h>`), resolved via the include directories exported by the
`pas_sys` CMake target. **No relative includes and no OS headers** appear in the
portable layer.

The single entry point is `sys.h`, which aggregates the following modules:

| Header          | Module                                      | Backend impl        |
|-----------------|---------------------------------------------|---------------------|
| `sys_types.h`   | Base types + platform-neutral error codes   | — (portable)        |
| `sys_err.h`     | Human-readable error strings                | `sys_err.c`         |
| `sys_mem.h`     | Raw memory allocation & platform properties | `*_alloc.c`         |
| `sys_file.h`    | Low-level file operations                   | `*_file.c`          |
| `sys_buf.h`     | Dynamic byte buffer + page-aligned I/O      | `sys_buf.c` / `*_buffer.c` |
| `sys_proc.h`    | Process start / wait / kill                 | `*_process.c`       |
| `sys_recv.h`    | TCP stream channels (`sys_chan_*`)          | `*_recv.c`          |

#### Error codes (`sys_types.h`)

`sys_err_t` is an `int`: `0` (`SYS_OK`) means success, negative values are
errors. The codes are deliberately **not** raw `errno`, so they are identical on
every OS; each backend maps the native error (`errno` / `GetLastError` /
`WSAGetLastError`) onto this table:

| Code                 | Value | Meaning                              |
|----------------------|-------|--------------------------------------|
| `SYS_OK`             | 0     | Success                              |
| `SYS_EINVAL`         | -1    | Invalid argument                     |
| `SYS_ENOENT`         | -2    | File / entry not found               |
| `SYS_EACCES`         | -3    | Permission denied                    |
| `SYS_EBADF`          | -4    | Invalid descriptor / handle          |
| `SYS_ENOMEM`         | -5    | Out of memory                        |
| `SYS_EEXIST`         | -6    | Entry already exists                 |
| `SYS_EIO`            | -7    | Generic I/O error                    |
| `SYS_EAGAIN`         | -8    | Operation would block (`SYS_EWOULDBLOCK` alias) |
| `SYS_ENOTSUP`        | -9    | Not supported by this backend        |
| `SYS_EPIPE`          | -10   | Channel closed by peer               |
| `SYS_ENOTCONN`       | -11   | Not connected                        |
| `SYS_ECONNREFUSED`   | -12   | Connection refused                   |
| `SYS_ETIMEDOUT`      | -13   | Timed out                            |
| `SYS_ECONNRESET`     | -14   | Connection reset by peer             |
| `SYS_EPROTO`         | -15   | Network protocol error               |

---

### Build

```bash
# Configure + build (out-of-source build in ./build)
cmake -S . -B build
cmake --build build

# Optional: install to the system prefix
cmake --install build
```

At configure time CMake prints the selected backend, e.g.:

```
-- Qt6 (6.4.2)
-- unix (Default)
```

### Run

```bash
./build/base_window
```

A 1000×505 frameless window appears on a teal desktop background:

- **Drag** the blue title bar to move the window.
- **Click** the `X` button to close it.

### Known Issues

1. **Implicit declaration of `rename`** (`source/os/unix/unix_file.c:199`)
   A compiler warning (`-Wimplicit-function-declaration`) is emitted because the
   POSIX `rename` declaration is not visible under the default feature-test
   macros. The code compiles and links, but the warning should be silenced by
   defining an appropriate feature-test macro (e.g. `_POSIX_C_SOURCE`) or by
   including the proper header before use.

2. **`conf/` and `media/` are runtime resources**
   They are copied to the build directory automatically after every build
   (`POST_BUILD` step) and also to `${CMAKE_INSTALL_DATADIR}/base_window/` on
   install. Keep them next to the executable when distributing manually.

3. **Backends are mutually exclusive**
   Exactly one OS backend (`unix` or `win32`) is compiled, selected by the CMake
   platform detection. Code in `source/sys` must never depend on a specific
   backend.

---

## Detail for Developers

### How the abstraction works

1. `source/sys` defines the **contract**: headers with platform-neutral types,
   opaque handles (`sys_file_t`, `sys_chan_t`, `sys_proc_t`, `sys_buf_t`) and
   error codes.
2. `source/os/<platform>` provides the **implementation** for that contract.
   Opaque handles hide whether the underlying object is a POSIX file descriptor,
   a Windows `HANDLE`, a `SOCKET`, or a `pid_t`.
3. `CMakeLists.txt` picks **exactly one** backend and exports the include
   directories (`source/sys/include` and `source/os/<backend>/include`) as
   `PUBLIC` on the `pas_sys` target. Application code therefore only ever
   includes `<sys.h>` and friends — the backend is invisible to it.

### Adding a new platform backend

1. Create `source/os/<platform>/` with the five implementation files:
   - `<platform>_alloc.c`    → `sys_mem_*`
   - `<platform>_buffer.c`   → `sys_buf_alloc_pages` / `sys_buf_free_pages`
   - `<platform>_file.c`     → `sys_file_*`
   - `<platform>_process.c`  → `sys_proc_*`
   - `<platform>_recv.c`     → `sys_chan_*`
2. Add an `include/` folder with `os_config.h` and `os_core.h` for that platform.
3. Register the sources in `CMakeLists.txt` (mirroring the `unix`/`win32`
   blocks) and set `OS_BACKEND` accordingly.
4. Map the platform's native errors onto the `SYS_*` table in `sys_types.h`.

### Front end (`source/core/init.cpp`)

- `initializeSystemLayer()` — smoke test of the abstraction layer: creates a
  buffer, appends a message, frees it; aborts startup if any call fails.
- `class BaseWindow : public QWidget` — the Win95-style window:
  - frameless (`Qt::FramelessWindowHint`), fixed 1000×505 size,
  - custom `paintEvent` draws the classic grey beveled border,
  - title-bar mouse events implement window dragging,
  - blue title bar with white bold title and a beveled `X` close button
    (bevel reverses on press, mimicking the classic look).
- `main()` — creates the teal "desktop", centers and shows `BaseWindow`, then
  runs the Qt event loop.

### Coding conventions

- System headers are included with angle brackets: `#include <sys_mem.h>`.
- **Never** use relative includes (`../`) or `#include "include/..."`.
- Application code must stay backend-agnostic: check error codes against the
  `SYS_*` constants, never against raw `errno`/`WSAGetLastError`.
- Handles are opaque structs; always access them through the `sys_*` API.

---
