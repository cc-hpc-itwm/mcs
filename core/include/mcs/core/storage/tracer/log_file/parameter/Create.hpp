// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/serialization/Concepts.hpp>

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
    struct Implementation<core::storage::tracer::log_file::parameter::Create<Storage>>
  {
    using Type = core::storage::tracer::log_file::parameter::Create<Storage>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/Create.ipp"
