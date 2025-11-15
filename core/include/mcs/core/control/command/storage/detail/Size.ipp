// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

namespace mcs::core::control::command::storage
{
  constexpr Size::Response::Response
    ( core::storage::MaxSize max
    , core::memory::Size used
    ) noexcept
      : _max {max}
      , _used {used}
  {}
  constexpr auto Size::Response::max
    (
    ) const noexcept -> core::storage::MaxSize
  {
    return _max;
  }
  constexpr auto Size::Response::used
    (
    ) const noexcept -> core::memory::Size
  {
    return _used;
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "control::provider::segment::storage::Size::Response "
  , mcs::core::control::command::storage::Size::Response
  , _max
  , _used
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ3
  ( "control::provider::segment::storage::Size "
  , mcs::core::control::command::storage::Size
  , storage_id
  , storage_parameter_size_max
  , storage_parameter_size_used
  );
