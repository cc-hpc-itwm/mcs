# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_read
  PRIVATE ParseError.cpp
)
set_target_properties (mcs_util_read PROPERTIES POSITION_INDEPENDENT_CODE ON)
