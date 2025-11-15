// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::iov_backend
{
  struct Location
  {
    core::memory::Range _range;
    // for file I/O
    util::ASIO::AnyConnectable _storages_provider;
    SupportedStorageImplementations::ID _storage_implementation_id;
    core::storage::Parameter _parameter_file_read;
    core::storage::Parameter _parameter_file_write;
    // for standard transport
    util::ASIO::AnyConnectable _transport_provider;
    core::transport::Address _address;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( mcs::iov_backend::Location
      );
}
