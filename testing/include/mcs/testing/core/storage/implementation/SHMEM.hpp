// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/format.h>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/UniqID.hpp>
#include <string>

namespace mcs::testing::core::storage::implementation
{
  struct SHMEM
  {
    SHMEM() = default;

    // Use different id for multiple storages
    //
    SHMEM (std::string id);
    SHMEM (std::string id, mcs::core::storage::MaxSize);
    SHMEM (mcs::core::storage::MaxSize);

    using Storage = mcs::core::storage::implementation::SHMEM;

    [[nodiscard]] auto parameter_create
      (
      ) const -> typename Storage::Parameter::Create
      ;

    [[nodiscard]] auto parameter_segment_create
      (
      ) const -> typename Storage::Parameter::Segment::Create
      ;
    [[nodiscard]] auto parameter_segment_remove
      (
      ) const -> typename Storage::Parameter::Segment::Remove
      ;

    [[nodiscard]] auto parameter_size_max
      (
      ) const -> typename Storage::Parameter::Size::Max
      ;
    [[nodiscard]] auto parameter_size_used
      (
      ) const -> typename Storage::Parameter::Size::Used
      ;

    [[nodiscard]] auto parameter_chunk_description
      (
      ) const -> typename Storage::Parameter::Chunk::Description
      ;

    [[nodiscard]] auto parameter_file_read
      (
      ) const -> typename Storage::Parameter::File::Read
      ;

    [[nodiscard]] auto parameter_file_write
      (
      ) const -> typename Storage::Parameter::File::Write
      ;

  private:
    std::string _id {"0"};
    UniqID _uniq_id
      { fmt::format ( "{}-{}"
                    , "TESTING-MCS-CORE-STORAGE-IMPLEMENTATION-SHMEM"
                    , _id
                    )
      };
    typename Storage::Prefix _prefix {util::string {_uniq_id()}};
    mcs::core::storage::MaxSize _max_size
      {mcs::core::storage::MaxSize::Unlimited{}};
  };
}
