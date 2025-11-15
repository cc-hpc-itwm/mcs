// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/file/Read.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION6
  ( mcs::core::control::command::file::Read
  , _storage_id
  , _parameter_file_read
  , _segment_id
  , _offset
  , _file
  , _range
  );
