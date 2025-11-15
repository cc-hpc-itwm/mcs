# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_block_device
  PRIVATE Storage.cpp
  PRIVATE block/Count.cpp
  PRIVATE block/ID.cpp
  PRIVATE block/Range.cpp
  PRIVATE block/Size.cpp
  PRIVATE meta_data/Blocks.cpp
  PRIVATE meta_data/command/Add.cpp
  PRIVATE meta_data/command/Location.cpp
  PRIVATE meta_data/command/Remove.cpp
  PRIVATE meta_data/provider/Handler.cpp
  PRIVATE storage/with_range.cpp
)
