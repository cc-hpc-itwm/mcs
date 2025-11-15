// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/FMT/declare.hpp>

namespace mcs::core::storage::trace::event::file
{
  template<core::storage::is_implementation Storage>
    struct Read
  {
    typename Storage::Parameter::File::Read _parameter_file_read;
    core::storage::segment::ID _segment_id;
    core::memory::Offset _offset;
    std::filesystem::path _path;
    core::memory::Range _range;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::trace::event::file::Read<Storage>
      );
}

#include "detail/Read.ipp"
