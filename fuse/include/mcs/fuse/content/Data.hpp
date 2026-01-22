// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace mcs::fuse::content
{
  // Collection of spans of data. Maintains the sum of the sizes.
  //
  struct Data
  {
    [[nodiscard]] constexpr Data() = default;

    [[nodiscard]] Data (std::span<std::byte const> chunk)
      : _size {chunk.size()}
      , _chunks {chunk}
    {}

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t
    {
      return _size;
    }

    auto emplace (std::span<std::byte const> chunk) -> void
    {
      _size += _chunks.emplace_back (chunk).size();
    }

    [[nodiscard]] constexpr auto chunks
      (
      ) const noexcept -> std::vector<std::span<std::byte const>> const&
    {
      return _chunks;
    }

    template<typename Fun>
      requires (std::invocable<Fun, std::span<std::byte const>>)
      auto for_each_chunk (Fun&& fun) const
    {
      std::ranges::for_each (_chunks, std::forward<Fun> (fun));
    }

  private:
    std::size_t _size {0UL};
    std::vector<std::span<std::byte const>> _chunks{};
  };
}
