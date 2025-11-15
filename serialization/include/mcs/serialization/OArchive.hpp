// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/detail/Tag.hpp>
#include <memory>
#include <span>
#include <vector>

namespace mcs::serialization
{
  // non-owning
  struct [[nodiscard]] OArchive
  {
    OArchive() = default;

    template<is_serializable... T>
      constexpr explicit OArchive (T const&...);

    template<is_serializable... T>
      constexpr explicit OArchive (T&...);

    template<is_serializable... T>
      constexpr explicit OArchive (T&&...);

    // Returns: The serialized bytes without size prefix.
    //
    // It holds:
    // bytes().size() + buffers().front().size() == sum_size_buffers()
    //
    // Complexity: O (buffers().size())
    //
    [[nodiscard]] auto bytes() const -> std::vector<std::byte>;

    // Iterates in order over all data buffers and calls back for
    // every buffer. Data buffers are all buffers except the buffer
    // for the size prefix.
    //
    template<typename Fun>
      requires (std::invocable<Fun, std::span<std::byte const>>)
      auto for_each_data_buffer (Fun&&) const -> void;

    using Buffers = std::vector<std::span<std::byte const>>;

    // Returns: All buffers, including the size prefix. The first
    // buffer is the "size prefix" and contains the sum of the
    // "appended" buffers. It does not include the sizes of the
    // "stream"ed buffers.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] constexpr auto buffers() const -> Buffers const&;

    // Returns: The sum of all buffer sizes, including the "stream"ed
    // buffers and including the size of the size prefix buffer.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] constexpr auto sum_size_buffers
      (
      ) const noexcept -> std::size_t;

    // OArchives take addresses of members.
    //
    OArchive (OArchive const&) = delete;
    auto operator= (OArchive const&) -> OArchive& = delete;
    OArchive (OArchive&&) = delete;
    auto operator= (OArchive&&) -> OArchive& = delete;
    ~OArchive() = default;

  private:
    template<typename> friend struct Implementation;

    template<typename TagType, typename... TagArgs>
      constexpr auto tag
        ( TagArgs&&...
        ) -> OArchive&
      ;

    template<typename T> auto append (std::span<T>) -> OArchive&;
    template<typename T> auto stream (std::span<T>) -> OArchive&;

    // Takes ownership of the bytes of the other archive.
    auto store (OArchive const&) -> OArchive&;

    template<is_serializable T>
      friend constexpr auto save (OArchive&, T const&) -> OArchive&;
    template<is_serializable T>
      friend constexpr auto save (OArchive&, T&) -> OArchive&;
    template<is_serializable T>
      friend auto save (OArchive&, T&&) -> OArchive&;

    template<typename T> auto emplace_back (std::span<T>) -> std::size_t;

    std::size_t _size_append {0};
    Buffers _buffers
      { std::as_bytes (std::span {std::addressof (_size_append), 1})
      };
    std::size_t _sum_size_buffers {_buffers.front().size()};
    std::list<std::vector<std::byte>> _bytes_for_temporaries;
  };
}

namespace mcs::serialization
{
  auto operator<< (std::ostream&, OArchive const&) -> std::ostream&;
}

#include "detail/OArchive.ipp"
