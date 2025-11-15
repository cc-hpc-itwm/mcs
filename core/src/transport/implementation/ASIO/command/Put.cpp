// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/ASIO/command/Put.hpp>
#include <mcs/serialization/define.hpp>

namespace mcs::core::transport::implementation::ASIO::command
{
  Put::Put
    ( core::transport::Address destination_
    , Bytes bytes
    )
      : destination {destination_}
      , bytes_or_size {bytes}
  {}

  Put::Put
    ( core::transport::Address destination_
    , std::size_t size
    )
      : destination {destination_}
      , bytes_or_size {size}
  {}
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , put
    , core::transport::implementation::ASIO::command::Put
    )
  {
    namespace ASIO = core::transport::implementation::ASIO;
    using Put = ASIO::command::Put;

    MCS_SERIALIZATION_SAVE_FIELD (oa, put, destination);

    // when saving, the bytes must be available
    auto bytes {std::get<typename Put::Bytes> (put.bytes_or_size)};

    // save the number of bytes in the archive and stream the bytes
    // through the channel
    save (oa, bytes.size());
    oa.stream (std::span {bytes.data(), bytes.size()});

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , core::transport::implementation::ASIO::command::Put
    )
  {
    namespace ASIO = core::transport::implementation::ASIO;
    using Put = ASIO::command::Put;

    MCS_SERIALIZATION_LOAD_FIELD (ia, destination, Put);

    // to restore read the number of bytes but do not extract the
    // bytes from the channel, that is the task of the command
    // handler. to have this separation enables the command handler to
    // prepare a local buffer with the appropriate size to store all
    // the bytes from the channel.
    auto size {load<std::size_t> (ia)};

    return Put {destination, size};
  }
}
