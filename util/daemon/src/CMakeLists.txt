# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_daemon
  PRIVATE daemon.cpp
)
set_target_properties (mcs_util_daemon PROPERTIES POSITION_INDEPENDENT_CODE ON)
