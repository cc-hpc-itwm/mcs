// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::control::command::file
{
  struct Read
  {
    using Response = mcs::core::memory::Size;

    storage::ID _storage_id;
    storage::Parameter _parameter_file_read;
    storage::segment::ID _segment_id;
    memory::Offset _offset;
    std::filesystem::path _file;
    memory::Range _range;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::file::Read);

#include "detail/Read.ipp"
