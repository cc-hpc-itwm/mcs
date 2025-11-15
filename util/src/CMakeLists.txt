# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util
  PRIVATE Copy.cpp
  PRIVATE DLHandle.cpp
  PRIVATE ExistingPath.cpp
  PRIVATE NonExistingPath.cpp
  PRIVATE OpenFile.cpp
  PRIVATE TemporaryDirectory.cpp
  PRIVATE TemporaryFile.cpp
  PRIVATE buffer/Bytes.cpp
  PRIVATE divru.cpp
  PRIVATE fopen.cpp
  PRIVATE read_file.cpp
  PRIVATE select.cpp
  PRIVATE touch.cpp
  PRIVATE write_file.cpp
)
target_link_libraries (mcs_util
  PRIVATE mcs_util_syscall
  PRIVATE rt
  PRIVATE ${CMAKE_DL_LIBS}
)
set_target_properties (mcs_util PROPERTIES POSITION_INDEPENDENT_CODE ON)
