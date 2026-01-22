// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::core::memory
{
  Range::Error::BeginMustNotBeLargerThanEnd::BeginMustNotBeLargerThanEnd
    ( Offset begin
    , Offset end
    ) noexcept
      : mcs::Error
        { fmt::format ( "memory::Range::BeginMustNotBeLargerThanEnd: {} > {}"
                      , begin
                      , end
                      )
        }
      , _begin {begin}
      , _end {end}
  {}
  Range::Error::BeginMustNotBeLargerThanEnd::~BeginMustNotBeLargerThanEnd()
    = default
    ;
}

namespace mcs::serialization
{
  auto Implementation<mcs::core::memory::Range>::output
    ( OArchive& oa
    , mcs::core::memory::Range const& value
    ) -> OArchive&
  {
    save (oa, value._begin);
    save (oa, value._end);

    return oa;
  }
  auto Implementation<mcs::core::memory::Range>::input
    ( IArchive& ia
    ) -> mcs::core::memory::Range
  {
    auto _begin {load<decltype (mcs::core::memory::Range::_begin)> (ia)};
    auto _end {load<decltype (mcs::core::memory::Range::_end)> (ia)};

    return mcs::core::memory::Range {_begin, _end};
  }
}
