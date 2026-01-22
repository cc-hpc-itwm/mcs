// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::core::storage::implementation::trace::parameter
{
  // To create the Trace implementation, both must be created: The
  // underlying storage and the tracer.
  //
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    struct Create
  {
    typename Tracer::Parameter::Create _parameter_tracer_create;
    typename Storage::Parameter::Create _parameter_storage_create;
  };
}

namespace mcs::serialization
{
  template<typename Tracer, core::storage::is_implementation Storage>
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    struct Implementation<core::storage::implementation::trace::parameter::Create<Tracer, Storage>>
  {
    using Type = core::storage::implementation::trace::parameter::Create<Tracer, Storage>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    struct formatter<mcs::core::storage::implementation::trace::parameter::Create<Tracer, Storage>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::trace::parameter::Create<Tracer, Storage> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Create.ipp"
