// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/util/tuplish/define.hpp>

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

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::memory::Range
  , _begin
  , _end
  );
