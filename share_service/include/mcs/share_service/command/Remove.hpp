// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
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
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (share_service::command::Remove)
    ;

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (share_service::command::remove::Parameters<StorageImplementation>)
    ;
}

#include "detail/Remove.ipp"
