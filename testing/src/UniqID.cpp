// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <bit>
#include <cstdint>
#include <fmt/format.h>
#include <mcs/testing/UniqID.hpp>
#include <mcs/util/cast.hpp>
#include <ranges>

namespace mcs::testing
{
  auto UniqID::Digest::operator<< (std::string_view value) -> Digest&
  {
    auto const length {util::cast<std::uint64_t> (value.size())};

    auto const update
      { [this] (std::uint8_t byte) noexcept
        {
          _laneA = (_laneA ^ byte) * UINT64_C (0x100000001b3);
          _laneB = (_laneB ^ byte) * UINT64_C (0x100000001b3);
        }
      };

    std::ranges::for_each
      ( std::views::iota (std::size_t{}, sizeof (length))
      , [&] (std::size_t i)
        {
          update
            ( util::cast<std::uint8_t> ((length >> (i * 8)) & UINT64_C (0xff))
            );
        }
      );

    std::ranges::for_each
      ( value
      , [&] (char character) noexcept
        {
          update (std::bit_cast<std::uint8_t> (character));
        }
      );

    return *this;
  }

  auto UniqID::operator()() const -> std::string
  {
    return _id;
  }

  auto UniqID::digest() const noexcept -> Digest
  {
    return _digest;
  }
}
