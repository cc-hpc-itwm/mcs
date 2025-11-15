# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

find_package (Threads REQUIRED)

include (FetchContent)

FetchContent_Declare (asio
  GIT_REPOSITORY  https://github.com/chriskohlhoff/asio.git
  GIT_TAG         asio-1-36-0
  GIT_SHALLOW     TRUE
  SOURCE_DIR      ${CMAKE_BINARY_DIR}/3rdparty/asio
  OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable (asio)

add_library (asio INTERFACE)
target_include_directories (asio
  SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include
)
target_link_libraries (asio INTERFACE Threads::Threads)
target_compile_definitions (asio INTERFACE ASIO_STANDALONE)
