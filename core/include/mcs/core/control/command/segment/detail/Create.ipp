// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_FMT_READ3
  ( "control::provider::segment::Create "
  , mcs::core::control::command::segment::Create
  , storage_id
  , storage_parameter
  , memory_size
  );
