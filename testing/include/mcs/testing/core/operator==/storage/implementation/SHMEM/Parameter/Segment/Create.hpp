// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <tuple>
#include <type_traits>

namespace mcs::core::storage::implementation
{
  constexpr auto operator==
    ( SHMEM::Parameter::Segment::AccessMode const& lhs
    , SHMEM::Parameter::Segment::AccessMode const& rhs
    ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x.value);
        }
      };

    return essence (lhs) == essence (rhs);
  }

  constexpr auto operator==
    ( SHMEM::Parameter::Segment::MLOCKed const&
    , SHMEM::Parameter::Segment::MLOCKed const&
    ) noexcept -> bool
  {
    static_assert (std::is_empty_v<SHMEM::Parameter::Segment::MLOCKed>);

    return true;
  }

  constexpr auto operator==
    ( SHMEM::Parameter::Segment::Create const& lhs
    , SHMEM::Parameter::Segment::Create const& rhs
    ) noexcept -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie ( x.access_mode
                          , x.mlocked
                          );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}
