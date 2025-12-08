// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <rdma/fi_errno.h>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail
{
  template <typename Fun, typename... Args>
    requires (  std::invocable<Fun, Args...>
             && std::is_same_v<std::invoke_result_t<Fun, Args...>, int>
             )
    auto invoke_and_throw_on_error
      ( Fun&& fun
      , error::LibfabricError::FunctionName function_name
      , Args&&... args
      ) -> void
  {
    auto const err
      { std::invoke
        ( std::forward<Fun> (fun)
        , std::forward<Args> (args)...
        )
      };

    if (err != 0)
    {
      throw error::LibfabricError
        { function_name
        , error::LibfabricError::ErrorName {fi_strerror (err)}
        };
    }
  }
}
