// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <fmt/format.h>
#include <functional>
#include <mcs/Error.hpp>
#include <memory>
#include <type_traits>
#include <utility>

namespace mcs::fuse::detail
{
  // Manage a resource that has create and destroy functions.
  //
  template<typename Description, typename Create, typename... Args>
    requires (  std::invocable<Create, Args...>
             && std::is_pointer_v<std::invoke_result_t<Create, Args...>>
             )
    auto managed_not_null_resource
      ( void (*destroy) (std::invoke_result_t<Create, Args...>)
      , Description description
      , Create&& create
      , Args&&... args
      )
  {
    using T = std::remove_pointer_t<std::invoke_result_t<Create, Args...>>;

    return std::unique_ptr<T, void (*) (T*)>
      { std::invoke
        ( [&]
          {
            auto* x
              { std::invoke
                ( std::forward<Create> (create)
                , std::forward<Args> (args)...
                )
              };

            if (!x)
            {
              throw mcs::Error
                { fmt::format ("{} returned nullptr", description)
                };
            }

            return x;
          }
        )
      , destroy
      };
  }
}
