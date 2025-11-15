// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <stdexcept>

namespace mcs::util
{
  template<typename T>
    constexpr auto Select::subspan
      ( std::span<T> data
      , typename std::span<T>::size_type offset
      , typename std::span<T>::size_type count
      ) -> decltype (data.subspan (offset, count))
  {
    if (offset > data.size() || count > data.size() - offset)
    {
      throw Error::OutOfRange {offset, count, data.size()};
    }

    return data.subspan (offset, count);
  }

  template<typename T>
    constexpr auto select
      ( std::span<T> data
      , typename std::span<T>::size_type offset
      , typename std::span<T>::size_type count
      ) -> decltype (data.subspan (offset, count))
  {
    return Select{}.subspan (data, offset, count);
  }
}

namespace mcs::util
{
  constexpr auto Select::Error::OutOfRange::offset
    (
    ) const noexcept -> std::size_t
  {
    return _offset;
  }

  constexpr auto Select::Error::OutOfRange::count
    (
    ) const noexcept -> std::size_t
  {
    return _count;
  }

  constexpr auto Select::Error::OutOfRange::size
    (
    ) const noexcept -> std::size_t
  {
    return _size;
  }
}
