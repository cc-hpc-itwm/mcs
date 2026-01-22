// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>

namespace mcs::share_service::command
{
  namespace remove
  {
    template<is_supported_storage_implementation StorageImplementation>
      struct Parameters
    {
      using Storage = StorageImplementation;
      using Parameter = typename StorageImplementation::Parameter;
      Parameter::Segment::Remove segment_remove;
    };
  }

  struct Remove
  {
    using Response = void;

    using Parameters
      = SupportedStorageImplementations::fmap<remove::Parameters>
      ;

    Chunk chunk;
    typename Parameters::Variant parameters;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<share_service::command::Remove>
  {
    using Type = share_service::command::Remove;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    struct Implementation<share_service::command::remove::Parameters<StorageImplementation>>
  {
    using Type = share_service::command::remove::Parameters<StorageImplementation>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/Remove.ipp"
