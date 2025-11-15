// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/core/Storages.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <span>
#include <variant>

namespace mcs::core
{
  namespace detail
  {
    template<chunk::is_access Access>
      struct Chunk
    {
      template<storage::is_implementation StorageImplementation>
        using State
          = StorageImplementation::Chunk::template Description<Access>::State
        ;
    };
  }

  template< chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    struct Chunk
  {
    explicit Chunk (chunk::Description<Access, StorageImplementations...>);

    [[nodiscard]] auto data() const -> typename Access::template Span<std::byte>;

  private:
    using States
      = util::type::List<StorageImplementations...>
        ::template fmap<detail::Chunk<Access>::template State>
      ;
    typename States::Variant _state;
  };
}

namespace mcs::core
{
  template< chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    [[nodiscard]] auto make_chunk
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
      , storage::ID
      , storage::Parameter parameter_chunk_description
      , storage::segment::ID
      , memory::Range
      ) -> Chunk<Access, StorageImplementations...>
    ;
}

namespace mcs::core
{
  template< typename T
          , chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    [[nodiscard]] auto as
      ( Chunk<Access, StorageImplementations...> const&
      ) -> typename Access::template Span<T>
    ;
}

#include "detail/Chunk.ipp"
