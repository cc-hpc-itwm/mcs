// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/segment/ID.hpp>

namespace mcs::core::storage::trace::event::file
{
  template<core::storage::is_implementation Storage>
    struct Write
  {
    typename Storage::Parameter::File::Write parameter_file_write;
    core::storage::segment::ID segment_id;
    core::memory::Offset offset;
    std::filesystem::path path;
    core::memory::Range range;
  };
}

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    struct formatter<mcs::core::storage::trace::event::file::Write<Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::file::Write<Storage> const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Write.ipp"
