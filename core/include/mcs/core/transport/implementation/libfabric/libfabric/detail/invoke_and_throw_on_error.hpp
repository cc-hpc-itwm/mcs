// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/core/transport/implementation/libfabric/error/LibfabricError.hpp>
#include <type_traits>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail
{
  template<typename Fun, typename... Args>
    requires (  std::invocable<Fun, Args...>
             && std::is_same_v<std::invoke_result_t<Fun, Args...>, int>
             )
    auto invoke_and_throw_on_error
      ( Fun&&
      , error::LibfabricError::FunctionName
      , Args&&...
      ) -> void
    ;
}

#include "detail/invoke_and_throw_on_error.ipp"
