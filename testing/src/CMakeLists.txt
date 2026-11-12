# Copyright (C) 2022-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_testing
  PRIVATE UniqID.cpp
  PRIVATE UniqTemporaryDirectory.cpp
  PRIVATE printer/STD/exception.cpp
  PRIVATE random/Test.cpp
  PRIVATE random/random_device.cpp
  PRIVATE random/seed.cpp
  PRIVATE random/value/CIdent.cpp
  PRIVATE random/value/bool.cpp
  PRIVATE random/value/UTIL/string.cpp
)

target_sources (mcs_testing_syscall
  PRIVATE syscall/_exit.cpp
  PRIVATE syscall/eventfd.cpp
  PRIVATE syscall/getrlimit.cpp
  PRIVATE syscall/kill.cpp
  PRIVATE syscall/setrlimit.cpp
  PRIVATE syscall/sigaction.cpp
  PRIVATE syscall/sigemptyset.cpp
  PRIVATE syscall/waitpid.cpp
  PRIVATE syscall/waitpid_retrying_on_interrupt.cpp
)
target_link_libraries (mcs_testing_syscall
  PUBLIC mcs_util_syscall_Error
  PRIVATE mcs_util
  PRIVATE mcs_util_syscall
)

add_subdirectory (RPC)

add_library (mcs_testing_block_device)
target_link_libraries (mcs_testing_block_device
  PUBLIC mcs_config
  PUBLIC mcs_testing
  PUBLIC mcs_block_device
)
add_subdirectory (block_device)

add_library (mcs_testing_core)
target_link_libraries (mcs_testing_core
  PUBLIC mcs_config
  PUBLIC mcs_testing
  PUBLIC mcs_core
)
add_subdirectory (core)
