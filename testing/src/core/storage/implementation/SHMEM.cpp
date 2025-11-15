// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <utility>

namespace mcs::testing::core::storage::implementation
{
  SHMEM::SHMEM (std::string id)
    : _id {std::move (id)}
  {}
  SHMEM::SHMEM (std::string id, mcs::core::storage::MaxSize max_size)
    : _id {std::move (id)}
    , _max_size {max_size}
  {}
  SHMEM::SHMEM (mcs::core::storage::MaxSize max_size)
    : _max_size {max_size}
  {}

  auto SHMEM::parameter_create
    (
    ) const -> typename Storage::Parameter::Create
  {
    return {_prefix, _max_size};
  }

  auto SHMEM::parameter_size_max
    (
    ) const -> typename Storage::Parameter::Size::Max
  {
    return {};
  }
  auto SHMEM::parameter_size_used
    (
    ) const -> typename Storage::Parameter::Size::Used
  {
    return {};
  }

  auto SHMEM::parameter_segment_create
    (
    ) const -> typename Storage::Parameter::Segment::Create
  {
    return {};
  }
  auto SHMEM::parameter_segment_remove
    (
    ) const -> typename Storage::Parameter::Segment::Remove
  {
    return {};
  }

  auto SHMEM::parameter_chunk_description
    (
    ) const -> typename Storage::Parameter::Chunk::Description
  {
    return {};
  }

  auto SHMEM::parameter_file_read
    (
    ) const -> typename Storage::Parameter::File::Read
  {
    return {};
  }

  auto SHMEM::parameter_file_write
    (
    ) const -> typename Storage::Parameter::File::Write
  {
    return {};
  }
}
