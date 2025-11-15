# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

include (FetchContent)

FetchContent_Declare (gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/3rdparty/googletest
  OVERRIDE_FIND_PACKAGE
)
set (INSTALL_GTEST OFF)
FetchContent_MakeAvailable (gtest)
include (GoogleTest)
