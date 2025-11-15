# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_error
  PRIVATE Error.cpp
)
set_target_properties (mcs_error PROPERTIES POSITION_INDEPENDENT_CODE ON)
