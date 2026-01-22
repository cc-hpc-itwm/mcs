// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <variant>

namespace mcs::share_service::command
{
  namespace attach
  {
    template<is_supported_storage_implementation StorageImplementation>
      struct Parameters
    {
      using Storage = StorageImplementation;
      using Parameter = typename StorageImplementation::Parameter;
      Parameter::Chunk::Description chunk_description;
    };
  }

  template<core::chunk::is_access Access>
    struct Attach
  {
    using Response
      = SupportedStorageImplementations::wrap<core::chunk::Description, Access>;

    using Parameters
      = SupportedStorageImplementations::fmap<attach::Parameters>
      ;

    Chunk chunk;
    typename Parameters::Variant parameters;
  };
}


namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    struct Implementation<share_service::command::Attach<Access>>
  {
    using Type = share_service::command::Attach<Access>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    struct Implementation<share_service::command::attach::Parameters<StorageImplementation>>
  {
    using Type = share_service::command::attach::Parameters<StorageImplementation>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/Attach.ipp"
