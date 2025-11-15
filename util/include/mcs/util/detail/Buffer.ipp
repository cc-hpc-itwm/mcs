// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>
#include <utility>

namespace mcs::util
{
  template<typename T, typename Deleter>
    template<typename... Args>
      Buffer<T, Deleter>::Buffer
        ( std::size_t size
        , Args&&... args
        )
    : _size {size}
    , _memory {std::forward<Args> (args)...}
  {}

  template<typename T, typename Deleter>
    constexpr auto Buffer<T, Deleter>::size() const noexcept -> std::size_t
  {
    return _size;
  }

  template<typename T, typename Deleter>
    template<typename U>
      auto Buffer<T, Deleter>::data() const -> std::span<U>
  {
    return {util::cast<U*> (_memory.get()), _size / sizeof (U)};
  }
}
