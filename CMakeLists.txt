cmake_minimum_required(VERSION 3.16)

project(base_window
    VERSION 0.1.0
    DESCRIPTION "Aplikasi Window Qt + SDL (gaya Windows 95)"
    LANGUAGES C CXX
)

# ---------------------------------------------------------------
# Standar C++ & otomatisasi Qt (moc/uic/rcc)
# ---------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------
# Cari Qt (utamakan Qt6, fallback ke Qt5)
# ---------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Menggunakan Qt6 (${Qt6_VERSION})")
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Menggunakan Qt5 (${Qt5_VERSION})")
endif()

# ---------------------------------------------------------------
# Cari SDL2
# ---------------------------------------------------------------
find_package(SDL2 REQUIRED)

# ---------------------------------------------------------------
# Lapisan abstraksi sistem (source/sys) + backend per-platform (source/os)
#
# source/sys  : API netral-platform (sys_mem_*, sys_file_*, sys_buf_*,
#               sys_proc_*, sys_chan_*) + logika portabel (sys_buf.c,
#               sys_err.c). TIDAK menyertakan header OS apa pun.
# source/os   : implementasi konkret; hanya SATU backend yang dipilih
#               sesuai sistem operasi target.
# ---------------------------------------------------------------
set(SYS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/source/sys)
set(OS_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/source/os)

set(SYS_SOURCES
    ${SYS_DIR}/sys_buf.c
    ${SYS_DIR}/sys_err.c
)

if(WIN32)
    set(OS_BACKEND win32)
    set(OS_SOURCES
        ${OS_DIR}/win32/win_alloc.c
        ${OS_DIR}/win32/win_buffer.c
        ${OS_DIR}/win32/win_file.c
        ${OS_DIR}/win32/win_process.c
        ${OS_DIR}/win32/win_recv.c
    )
else()
    set(OS_BACKEND unix)
    set(OS_SOURCES
        ${OS_DIR}/unix/unix_alloc.c
        ${OS_DIR}/unix/unix_buffer.c
        ${OS_DIR}/unix/unix_file.c
        ${OS_DIR}/unix/unix_process.c
        ${OS_DIR}/unix/unix_recv.c
    )
endif()
message(STATUS "Backend OS: ${OS_BACKEND}")

add_library(pas_sys STATIC ${SYS_SOURCES} ${OS_SOURCES})
target_include_directories(pas_sys PUBLIC
    ${SYS_DIR}/include
    ${OS_DIR}/${OS_BACKEND}/include
)
if(WIN32)
    target_link_libraries(pas_sys PUBLIC ws2_32)   # WinSock untuk sys_chan_*
endif()

# ---------------------------------------------------------------
# Target utama
# ---------------------------------------------------------------
add_executable(base_window
    source/core/init.cpp
)

target_link_libraries(base_window PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    SDL2::SDL2
    pas_sys
)

# Fallback jika SDL2 tidak menyediakan target imported SDL2::SDL2
if(NOT TARGET SDL2::SDL2)
    target_include_directories(base_window PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(base_window PRIVATE ${SDL2_LIBRARIES})
endif()

# ---------------------------------------------------------------
# Salin resource runtime (conf/ dan media/) ke folder build
# agar aplikasi menemukannya saat dijalankan dari build dir
# ---------------------------------------------------------------
foreach(_res_dir conf media)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_res_dir}")
        add_custom_command(TARGET base_window POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/${_res_dir}"
                    "$<TARGET_FILE_DIR:base_window>/${_res_dir}"
            COMMENT "Menyalin ${_res_dir}/ ke direktori build"
        )
    endif()
endforeach()

# ---------------------------------------------------------------
# Instalasi (opsional): make install / cmake --install build
# ---------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS base_window RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
foreach(_res_dir conf media)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_res_dir}")
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_res_dir}/"
                DESTINATION ${CMAKE_INSTALL_DATADIR}/base_window/${_res_dir})
    endif()
endforeach()