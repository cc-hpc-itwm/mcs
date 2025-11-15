// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Virtual/Parameter/Create.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  [[nodiscard]] auto operator==
    ( Virtual::Parameter::Create const& lhs
    , Virtual::Parameter::Create const& rhs
    ) -> bool
  {
    auto essence
      { [] (auto const& x)
        {
          return std::make_tuple
            ( x._shared_object
            , x._parameter_create
            );
        }
      };

    return essence (lhs) == essence (rhs);
  }
}
