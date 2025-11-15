// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::chunk
{
  namespace detail
  {
    template<is_access Access>
      struct Chunk
    {
      template<storage::is_implementation StorageImplementation>
        using Description
          = typename StorageImplementation::Chunk::template Description<Access>
        ;
    };
  }

  template< is_access Access
          , storage::is_implementation... StorageImplementations
          >
    using Descriptions = util::type::List<StorageImplementations...>
      ::template fmap<detail::Chunk<Access>::template Description>
    ;

  template< typename Access
          , typename D
          , typename... StorageImplementations
          >
    concept is_description
      = Descriptions<Access, StorageImplementations...>::template contains<D>()
    ;

  template< is_access Access
          , storage::is_implementation... StorageImplementations
          >
    using Description
      = typename Descriptions<Access, StorageImplementations...>::Variant
    ;
}
