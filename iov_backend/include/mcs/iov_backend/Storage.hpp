// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::iov_backend
{
  struct Storage
  {
    SupportedStorageImplementations::ID _storage_implementation_id;
    core::storage::Parameter _parameter_create;
    util::ASIO::AnyConnectable _storages_provider;
    util::ASIO::AnyConnectable _transport_provider;
    core::storage::ID _storage_id;
    core::storage::Parameter _parameter_size_max;
    core::storage::Parameter _parameter_size_used;
    core::storage::Parameter _parameter_segment_create;
    core::storage::Parameter _parameter_segment_remove;
    core::storage::Parameter _parameter_chunk_description;
    core::storage::Parameter _parameter_file_read;
    core::storage::Parameter _parameter_file_write;

    // \todo std::optional<core::storage::MaxSize> _exclusive;
    // describes a fraction of the storage that is used exclusively by
    // the iov_backend and all operations happen via the iov_backend
    // provider. if this is the case, then the capacity of the storage
    // can be maintained by the iov_backend provider and there is no
    // need to query the storage for its capacity.

#ifdef _cpp_lib_constexpr_vector
    constexpr
#endif
              auto operator<=> (Storage const&) const noexcept = default;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::iov_backend::Storage);

#include "detail/Storage.ipp"
