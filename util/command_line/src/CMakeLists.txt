# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_command_line
  PRIVATE arguments/Base.cpp
  PRIVATE arguments/Owned.cpp
  PRIVATE command_line/Parsed.cpp
  PRIVATE command_line/Schema.cpp
)
set_target_properties (mcs_util_command_line
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)
