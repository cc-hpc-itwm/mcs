// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <utility>

namespace mcs::testing::core::storage::implementation
{
  Files::Files (std::string id)
    : _id {std::move (id)}
  {}
  Files::Files (std::string id, mcs::core::storage::MaxSize max_size)
    : _id {std::move (id)}
    , _max_size {max_size}
  {}
  Files::Files (mcs::core::storage::MaxSize max_size)
    : _max_size {max_size}
  {}

  auto Files::parameter_create
    (
    ) const -> typename Storage::Parameter::Create
  {
    return {typename Storage::Prefix {_temp.path()}, _max_size};
  }

  auto Files::parameter_size_max
    (
    ) const -> typename Storage::Parameter::Size::Max
  {
    return {};
  }
  auto Files::parameter_size_used
    (
    ) const -> typename Storage::Parameter::Size::Used
  {
    return {};
  }

  auto Files::parameter_segment_create
    (
    ) const -> typename Storage::Parameter::Segment::Create
  {
    return {};
  }
  auto Files::parameter_segment_create
    ( typename Storage::Parameter::Segment::Persistency persistency
    ) const -> typename Storage::Parameter::Segment::Create
  {
    return typename Storage::Parameter::Segment::Create
      {std::move (persistency)};
  }
  auto Files::parameter_segment_remove
    (
    ) const -> typename Storage::Parameter::Segment::Remove
  {
    return {};
  }
  auto Files::parameter_segment_remove
    ( typename Storage::Parameter::Segment::ForceRemoval force_removal
    ) const -> typename Storage::Parameter::Segment::Remove
  {
    return typename Storage::Parameter::Segment::Remove
      {std::move (force_removal)};
  }

  auto Files::parameter_chunk_description
    (
    ) const -> typename Storage::Parameter::Chunk::Description
  {
    return {};
  }

  auto Files::parameter_file_read
    (
    ) const -> typename Storage::Parameter::File::Read
  {
    return {};
  }

  auto Files::parameter_file_write
    (
    ) const -> typename Storage::Parameter::File::Write
  {
    return {};
  }
}
