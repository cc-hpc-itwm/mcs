# Copyright (C) 2022-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_serialization
  PRIVATE IArchive.cpp
  PRIVATE OArchive.cpp
  PRIVATE error/AdditionalBytes.cpp
  PRIVATE error/Load.cpp
  PRIVATE error/NotEnoughBytes.cpp
  PRIVATE error/Save.cpp
  PRIVATE error/WrongTag.cpp
  PRIVATE STD/filesystem/path.cpp
)
target_link_libraries (mcs_serialization
  PRIVATE fmt
  PRIVATE mcs_util_FMT
)
set_target_properties (mcs_serialization
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)
