// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>

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
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      );
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      );
}

#include "detail/Create.ipp"
