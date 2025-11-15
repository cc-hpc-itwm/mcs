// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_FMT_READ6
  ( "control::provider::file::Write "
  , mcs::core::control::command::file::Write
  , _storage_id
  , _parameter_file_write
  , _segment_id
  , _offset
  , _file
  , _range
  );
