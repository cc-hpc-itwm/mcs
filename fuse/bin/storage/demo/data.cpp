// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "data.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>

namespace
{
  // A pattern function that generates reproducible data based on
  // offset.
  //
  auto pattern (std::size_t offset) -> std::byte
  {
    return static_cast<std::byte> ((offset * 17 + 42) & 0xFF);
  }
}

namespace mcs::fuse::demo
{
  auto generate (std::span<std::byte> buffer, std::size_t offset) -> void
  {
    std::ranges::generate
      ( buffer
      , [offset]() mutable noexcept { return pattern (offset++); }
      );
  }

  auto verify (std::span<std::byte const> buffer, std::size_t offset) -> void
  {
    std::ranges::for_each
      ( buffer
      , [offset] (std::byte actual) mutable
        {
          if ( auto const expected {pattern (offset)}
             ; actual != expected
             )
          {
            throw std::runtime_error
              { fmt::format
                ( "Pattern mismatch at file offset {}: expected '{}', got '{}'"
                , offset
                , expected
                , actual
                )
              };
          }

          ++offset;
        }
      );
  }
}
