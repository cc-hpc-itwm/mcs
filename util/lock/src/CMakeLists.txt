# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_lock
  PRIVATE queue/FIFO.cpp
  PRIVATE queue/Fast.cpp
)
set_target_properties (mcs_util_lock PROPERTIES POSITION_INDEPENDENT_CODE ON)
