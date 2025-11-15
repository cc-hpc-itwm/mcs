// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/storage/implementation/Heap.hpp>

namespace mcs::testing::core::storage::implementation
{
  Heap::Heap (std::string)
  {}
  Heap::Heap (std::string, mcs::core::storage::MaxSize max_size)
    : _max_size {max_size}
  {}
  Heap::Heap (mcs::core::storage::MaxSize max_size)
    : _max_size {max_size}
  {}

  auto Heap::parameter_create
    (
    ) const -> typename Storage::Parameter::Create
  {
    return {_max_size};
  }

  auto Heap::parameter_size_max
    (
    ) const -> typename Storage::Parameter::Size::Max
  {
    return {};
  }
  auto Heap::parameter_size_used
    (
    ) const -> typename Storage::Parameter::Size::Used
  {
    return {};
  }

  auto Heap::parameter_segment_create
    (
    ) const -> typename Storage::Parameter::Segment::Create
  {
    return {};
  }
  auto Heap::parameter_segment_remove
    (
    ) const -> typename Storage::Parameter::Segment::Remove
  {
    return {};
  }

  auto Heap::parameter_chunk_description
    (
    ) const -> typename Storage::Parameter::Chunk::Description
  {
    return {};
  }

  auto Heap::parameter_file_read
    (
    ) const -> typename Storage::Parameter::File::Read
  {
    return {};
  }

  auto Heap::parameter_file_write
    (
    ) const -> typename Storage::Parameter::File::Write
  {
    return {};
  }
}
