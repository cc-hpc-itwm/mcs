// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/buffer/Bytes.hpp>

namespace mcs::util::buffer
{
  Bytes::Bytes (std::size_t size)
    : Buffer<std::byte[]>
      { size
      , std::make_unique_for_overwrite<std::byte[]> (size)
      }
  {}
}
