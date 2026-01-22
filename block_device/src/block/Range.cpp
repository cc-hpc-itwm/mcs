// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/block_device/block/Range.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::block::Range>::output
    ( OArchive& oa
    , mcs::block_device::block::Range const& value
    ) -> OArchive&
  {
    save (oa, value._begin);
    save (oa, value._end);

    return oa;
  }
  auto Implementation<mcs::block_device::block::Range>::input
    ( IArchive& ia
    ) -> mcs::block_device::block::Range
  {
    auto _begin {load<decltype (mcs::block_device::block::Range::_begin)> (ia)};
    auto _end {load<decltype (mcs::block_device::block::Range::_end)> (ia)};

    return mcs::block_device::block::Range {_begin, _end};
  }
}

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
