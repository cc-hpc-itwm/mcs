# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

include (FetchContent)

FetchContent_Declare (fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        12.1.0
  GIT_SHALLOW    TRUE
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/3rdparty/fmt
  OVERRIDE_FIND_PACKAGE
)
set (FMT_DOC OFF)
set (FMT_INSTALL OFF)
set (FMT_TEST OFF)
set (FMT_FUZZ OFF)
set (FMT_CUDA_TEST OFF)
set (FMT_OS ON)
set (FMT_MODULE OFF)
set (FMT_SYSTEM_HEADERS ON)
set (FMT_UNICODE ON)
FetchContent_MakeAvailable (fmt)
target_compile_definitions (fmt PUBLIC FMT_HEADER_ONLY)
