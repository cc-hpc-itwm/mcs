// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/operator==/storage/implementation/SHMEM/Prefix.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( typename SHMEM::Parameter::Create const& lhs
    , typename SHMEM::Parameter::Create const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.prefix, x.max_size);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}
