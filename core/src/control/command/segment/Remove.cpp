// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/segment/Remove.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3
  ( mcs::core::control::command::segment::Remove
  , storage_id
  , storage_parameter
  , segment_id
  );
