// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <memory>
#include <span>

namespace mcs::util
{
  // owning
  template<typename T, typename Deleter = std::default_delete<T>>
    struct Buffer
  {
    using Memory = std::unique_ptr<T, Deleter>;

    constexpr Buffer() noexcept = default;

    template<typename... Args>
      explicit Buffer
        ( std::size_t
        , Args&&...
        )
      ;

    template<typename U> [[nodiscard]] auto data() const -> std::span<U>;

    constexpr auto size() const noexcept -> std::size_t;

  private:
    std::size_t _size {0};
    Memory _memory {nullptr};
  };
}

#include "detail/Buffer.ipp"
