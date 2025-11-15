# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_core
  PRIVATE control/command/file/Read.cpp
  PRIVATE control/command/file/Write.cpp
  PRIVATE control/command/segment/Create.cpp
  PRIVATE control/command/segment/Remove.cpp
  PRIVATE control/command/storage/Remove.cpp
  PRIVATE control/command/storage/Size.cpp
  PRIVATE control/command/storage/size/Max.cpp
  PRIVATE control/command/storage/size/Used.cpp
  PRIVATE memory/Offset.cpp
  PRIVATE memory/Range.cpp
  PRIVATE memory/Size.cpp
  PRIVATE segment/ID.cpp
  PRIVATE storage/ID.cpp
  PRIVATE storage/Implement_C_API.cpp
  PRIVATE storage/MaxSize.cpp
  PRIVATE storage/Parameter.cpp
  PRIVATE storage/implementation/Files.cpp
  PRIVATE storage/implementation/Heap.cpp
  PRIVATE storage/implementation/Import_C_API.cpp
  PRIVATE storage/implementation/SHMEM.cpp
  PRIVATE storage/implementation/Virtual.cpp
  PRIVATE transport/Address.cpp
  PRIVATE transport/client/ID.cpp
  PRIVATE transport/implementation/ASIO/command/Get.cpp
  PRIVATE transport/implementation/ASIO/command/Put.cpp
)
target_link_libraries (mcs_core
  PRIVATE mcs_nonstd_scope
  PRIVATE mcs_util_string
)
set_target_properties (mcs_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
