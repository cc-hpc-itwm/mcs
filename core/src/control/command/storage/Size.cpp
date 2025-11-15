// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/storage/Size.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::control::command::storage::Size::Response
  , _max
  , _used
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3
  ( mcs::core::control::command::storage::Size
  , storage_id
  , storage_parameter_size_max
  , storage_parameter_size_used
  );
