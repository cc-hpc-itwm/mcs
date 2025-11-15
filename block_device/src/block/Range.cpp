// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/block_device/block/Range.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::block_device::block::Range
  , _begin
  , _end
  );

namespace mcs::block_device::block
{
  Range::Error::BeginMustBeSmallerThanEnd::BeginMustBeSmallerThanEnd
    ( ID begin
    , ID end
    )
      : mcs::Error
        { fmt::format
          ( "mcs::block_device::block::Range::Error::BeginMusBeSmallerThanEnd:"
            " ! ({} < {})"
          , begin
          , end
          )
        }
      , _begin {begin}
      , _end {end}
  {}
  Range::Error::BeginMustBeSmallerThanEnd::~BeginMustBeSmallerThanEnd() = default;
}
