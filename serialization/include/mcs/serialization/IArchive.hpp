// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>
#include <mcs/serialization/Concepts.hpp>
#include <span>
#include <type_traits>
#include <vector>

namespace mcs::serialization
{
  // non-owning
  struct [[nodiscard]] IArchive
  {
    using Buffer = std::span<std::byte const>;

    template<typename... BufferArgs>
      requires (std::is_constructible_v<Buffer, BufferArgs...>)
      constexpr explicit IArchive (BufferArgs&&...) noexcept;

    explicit IArchive (std::vector<std::byte> const&) noexcept;
    explicit IArchive (std::span<std::byte> const&) noexcept;

  private:
    template<is_serializable T, typename... BufferArgs>
      friend auto load_from (BufferArgs&&...) -> T;

    template<is_serializable T>
      friend constexpr auto load (IArchive&) -> T;

    template<typename> friend struct Implementation;

    template<is_serializable TagType>
      auto tag() -> TagType;

    template<typename To>
      auto extract (To, std::size_t) -> void;

    Buffer _buffer;
  };
}

#include "detail/IArchive.ipp"
