// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/select.hpp>

namespace mcs::util
{
  Select::Error::OutOfRange::OutOfRange
    ( std::size_t offset
    , std::size_t count
    , std::size_t size
    ) noexcept
      : mcs::Error
        { fmt::format ( "select: offset + count > data.size(): {} + {} > {}"
                      , offset
                      , count
                      , size
                      )
        }
      , _offset {offset}
      , _count {count}
      , _size {size}
  {}
  Select::Error::OutOfRange::OutOfRange::~OutOfRange() = default;
}
