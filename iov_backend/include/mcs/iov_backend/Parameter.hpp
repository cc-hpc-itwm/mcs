// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>

namespace mcs::iov_backend
{
  struct Parameter
  {
    util::ASIO::AnyConnectable _provider;

    struct NumberOfThreads
    {
      // The number of threads used for data transport.
      //
      rpc::ScopedRunningIOContext::NumberOfThreads _transport_clients {4u};
    };
    NumberOfThreads _number_of_threads{};

    struct IndirectCommunication
    {
      // Indirect communication copies data from user memory into a
      // communication buffer in order to write them to a remote
      // location. And, to read data, indirect communication first
      // reads from a remote location into a communication buffer and
      // then copies the received data into the user memory.
      //
      // The communication buffer consists of _number_of_buffers many
      // buffer of size _maximum_transfer_size.
      //
      // Messages transfer happens in parts with a size of at most
      // _maximum_transfer_size.
      //
      // If a buffer can not be acquired within the timeout, then the
      // communication fails.
      //
      std::size_t _number_of_buffers {1 << 10};
      core::memory::Size _maximum_transfer_size
        {core::memory::make_size (32 << 10)};
      std::size_t _acquire_buffer_timeout_in_milliseconds {10'0000};

      // Indirect communication executes at most
      // _maximum_number_of_parallel_streams at the same time. A
      // "stream" is a sequence of individual read or write operations
      // to a single storage provider.
      //
      std::size_t _maximum_number_of_parallel_streams {4};
    };
    // \todo split into indirect_read and indirect_write!?
    IndirectCommunication _indirect_communication{};

    struct DirectCommunication
    {
      // Direct communication transfers data directly between user
      // memory and remote locations. In order to do so the user
      // memory must be allocated using backend_allocate.
      //
      // Messages transfer happens in parts with a size of at most
      // _maximum_transfer_size.
      //
      core::memory::Size _maximum_transfer_size
        {core::memory::make_size (1 << 30)};

      // Direct communication executes at most
      // _maximum_number_of_parallel_streams at the same time. A
      // "stream" is a set of read or write operations to a single
      // storage provider.
      //
      std::size_t _maximum_number_of_parallel_streams {4};
    };
    // \todo split into direct_read and direct_write!?
    DirectCommunication _direct_communication{};

    // modify individual components
    [[nodiscard]] auto operator<< (util::ASIO::AnyConnectable) -> Parameter&;
    [[nodiscard]] auto operator<< (NumberOfThreads) -> Parameter&;
    [[nodiscard]] auto operator<< (IndirectCommunication) -> Parameter&;
    [[nodiscard]] auto operator<< (DirectCommunication) -> Parameter&;

    // \todo recursive individual modification for all components
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      ( mcs::iov_backend::Parameter
      );
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::Parameter
      );
}

#include "detail/Parameter.ipp"
