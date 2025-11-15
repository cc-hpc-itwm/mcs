// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/buffer.hpp>
#include <cstddef>
#include <cstdint>
#include <mcs/util/buffer/Bytes.hpp>

namespace mcs::rpc::detail
{
  //! \todo isn't that a serialization::Buffer?
  struct Buffer : public util::buffer::Bytes
  {
    using util::buffer::Bytes::Bytes;

    template<typename T>
      [[nodiscard]] auto load() -> T;

    [[nodiscard]] auto modifiable() const noexcept -> asio::mutable_buffer;
  };
}

#include "detail/Buffer.ipp"
