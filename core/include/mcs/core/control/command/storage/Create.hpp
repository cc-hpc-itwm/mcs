// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <mcs/util/type/ID.hpp>

namespace mcs::core::control::command::storage
{
  template<core::storage::is_implementation... StorageImplementations>
    struct Create
  {
    using Response = core::storage::ID;

    util::type::ID<StorageImplementations...> implementation_id;
    core::storage::Parameter storage_parameter;
  };

  template< core::storage::is_implementation StorageImplementation
          , core::storage::is_implementation... StorageImplementations
          >
    auto create
      ( typename StorageImplementation::Parameter::Create
      ) noexcept -> Create<StorageImplementations...>
    ;
}

namespace fmt
{
  template<mcs::core::storage::is_implementation... StorageImplementations>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::control::command::storage::Create<StorageImplementations...>
      );
}

namespace mcs::serialization
{
  template<core::storage::is_implementation... StorageImplementations>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::control::command::storage::Create<StorageImplementations...>
      );
}

namespace mcs::util::read
{
  template<core::storage::is_implementation... StorageImplementations>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::control::command::storage::Create<StorageImplementations...>
      );
}

#include "detail/Create.ipp"
