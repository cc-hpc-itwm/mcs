// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/STD/tuple.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <tuple>

namespace mcs::share_service::command
{
  namespace create
  {
    template<is_supported_storage_implementation StorageImplementation>
      struct Parameters
    {
      using Parameter = typename StorageImplementation::Parameter;
      Parameter::Create create;
      Parameter::Segment::Create segment_create;
    };
  }

  struct Create
  {
    // \todo struct, hide STL
    using Response = std::tuple < core::storage::ID
                                , SupportedStorageImplementations::ID
                                , core::storage::segment::ID
                                >;

    using Parameters
      = SupportedStorageImplementations::fmap<create::Parameters>
      ;

    core::memory::Size size;
    typename Parameters::Variant parameters;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (share_service::command::Create)
    ;

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (share_service::command::create::Parameters<StorageImplementation>)
    ;
}

#include "detail/Create.ipp"
