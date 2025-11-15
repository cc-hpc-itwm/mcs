// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::core::storage::tracer::log_file::parameter
{
  template<is_implementation Storage>
    struct Create
  {
    std::filesystem::path _path;
  };
}

namespace mcs::serialization
{
  template<core::storage::is_implementation Storage>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::tracer::log_file::parameter::Create<Storage>
      );
}

#include "detail/Create.ipp"
