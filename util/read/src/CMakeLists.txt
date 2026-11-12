# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_read
  PRIVATE ParseError.cpp
  PRIVATE split_at_last_whitespace.cpp
)
set_target_properties (mcs_util_read PROPERTIES POSITION_INDEPENDENT_CODE ON)
