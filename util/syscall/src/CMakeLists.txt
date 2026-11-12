# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_syscall_Error
  PRIVATE Error.cpp
)
set_target_properties (mcs_util_syscall_Error
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)

target_sources (mcs_util_syscall
  PRIVATE syscall/close.cpp
  PRIVATE syscall/copy_file_range.cpp
  PRIVATE syscall/copy_file_range_with_fallback_to_sendfile.cpp
  PRIVATE syscall/dlclose.cpp
  PRIVATE syscall/dlerror.cpp
  PRIVATE syscall/dlopen.cpp
  PRIVATE syscall/dlsym.cpp
  PRIVATE syscall/dup.cpp
  PRIVATE syscall/dup2.cpp
  PRIVATE syscall/fflush.cpp
  PRIVATE syscall/fileno.cpp
  PRIVATE syscall/fork.cpp
  PRIVATE syscall/fstat.cpp
  PRIVATE syscall/fstatat.cpp
  PRIVATE syscall/ftruncate.cpp
  PRIVATE syscall/getegid.cpp
  PRIVATE syscall/geteuid.cpp
  PRIVATE syscall/getgid.cpp
  PRIVATE syscall/getpid.cpp
  PRIVATE syscall/getuid.cpp
  PRIVATE syscall/getxattr.cpp
  PRIVATE syscall/hostname.cpp
  PRIVATE syscall/ioctl.cpp
  PRIVATE syscall/lseek.cpp
  PRIVATE syscall/mkdirat.cpp
  PRIVATE syscall/mlock.cpp
  PRIVATE syscall/mmap.cpp
  PRIVATE syscall/munlock.cpp
  PRIVATE syscall/munmap.cpp
  PRIVATE syscall/open.cpp
  PRIVATE syscall/openat.cpp
  PRIVATE syscall/pipe.cpp
  PRIVATE syscall/pwrite.cpp
  PRIVATE syscall/raise.cpp
  PRIVATE syscall/read.cpp
  PRIVATE syscall/read_retrying_on_interrupt.cpp
  PRIVATE syscall/realloc.cpp
  PRIVATE syscall/renameat.cpp
  PRIVATE syscall/renameat2.cpp
  PRIVATE syscall/sendfile.cpp
  PRIVATE syscall/setsid.cpp
  PRIVATE syscall/shm_open.cpp
  PRIVATE syscall/shm_unlink.cpp
  PRIVATE syscall/statfs.cpp
  PRIVATE syscall/strdup.cpp
  PRIVATE syscall/sysconf.cpp
  PRIVATE syscall/unlinkat.cpp
  PRIVATE syscall/write.cpp
  PRIVATE syscall/write_retrying_on_interrupt.cpp
)
target_link_libraries (mcs_util_syscall
  PRIVATE fmt
  PRIVATE mcs_util
)
set_target_properties (mcs_util_syscall PROPERTIES POSITION_INDEPENDENT_CODE ON)
