// Copyright (C) 2025 Fraunhofer ITWM
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
    typename Storage::Parameter::File::Write _parameter_file_write;
    core::storage::segment::ID _segment_id;
    core::memory::Offset _offset;
    std::filesystem::path _path;
    core::memory::Range _range;
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
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Write.ipp"
