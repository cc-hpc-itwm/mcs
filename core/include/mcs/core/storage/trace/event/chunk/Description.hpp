// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/segment/ID.hpp>

namespace mcs::core::storage::trace::event::chunk
{
  template< core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    struct Description
  {
    typename Storage::Parameter::Chunk::Description
      _parameter_chunk_description;
    core::storage::segment::ID _segment_id;
    core::memory::Range _memory_range;
  };
}

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    struct formatter<mcs::core::storage::trace::event::chunk::Description<Storage, Access>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::trace::event::chunk::Description<Storage, Access> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Description.ipp"
