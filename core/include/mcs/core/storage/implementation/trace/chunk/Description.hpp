// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <utility>

namespace mcs::core::storage::implementation::trace::chunk
{
  template< typename Tracer
          , is_implementation Storage
          , core::chunk::is_access Access
          >
    requires (storage::trace::is_tracer<Tracer, Storage>)
    struct Description : public Storage::Chunk::template Description<Access>
    {
      using Base = typename Storage::Chunk::template Description<Access>;
      Description (Base&& base) : Base {std::forward<Base> (base)} {}

      struct State : public Storage::Chunk::template Description<Access>::State
      {
        using Base = typename Storage::Chunk::template Description<Access>::State;
        using Base::bytes;

        explicit State (Description const& description)
          : Base {static_cast<typename Description::Base> (description)}
        {}
      };
    };
}

namespace mcs::serialization
{
  template< typename Tracer
          , core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      );
}

namespace fmt
{
  template< typename Tracer
          , mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      );
}

#include "detail/Description.ipp"
