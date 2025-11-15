// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <mcs/core/memory/Size.hpp>
#include <stdexcept>

namespace mcs::core::transport::implementation::ASIO::command
{
  template<typename Socket>
    auto Get::stream (Socket& socket) const -> void
  {
    auto data {destination->data()};
    auto const bytes_read
      {memory::make_size (asio::read (socket, asio::mutable_buffer (data)))};

    if (bytes_read != size)
    {
      throw typename Error::CouldNotReadAllData
        { typename Error::CouldNotReadAllData::Wanted {size}
        , typename Error::CouldNotReadAllData::Read {bytes_read}
        };
    }
  }

  constexpr Get::Error::CouldNotReadAllData::Wanted::Wanted
    ( mcs::core::memory::Size value_
    ) noexcept
      : value {value_}
  {}
  constexpr Get::Error::CouldNotReadAllData::Read::Read
    ( mcs::core::memory::Size value_
    ) noexcept
      : value {value_}
  {}
  constexpr auto Get::Error::CouldNotReadAllData::wanted
    (
    ) const noexcept -> Wanted
  {
    return _wanted;
  }
  constexpr auto Get::Error::CouldNotReadAllData::read
    (
      ) const noexcept -> Read
  {
    return _read;
  }
}
