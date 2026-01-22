// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <exception>
#include <functional>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <memory>
#include <utility>

namespace mcs::fuse::detail
{
  // Manage a resource that has a constructor and a destroy function.
  //
  template<typename T, typename... Args>
    requires (std::is_constructible_v<T, Args...>)
    auto managed_object
      ( void (*destroy) (T*)
      , Args&&... args
      )
  {
    struct Deleter
    {
      auto operator() (T* x) const noexcept
      {
        try
        {
          std::invoke (_destroy, x);
        }
        catch (...)
        {
          util::FMT::print_noexcept
            ( stderr
            , "managed_object::Deleter::destroy failed: {}"
            , std::current_exception()
            );
        }

        std::default_delete<T>{} (x);
      }
      void (*_destroy) (T*);
    };

    return std::unique_ptr<T, Deleter>
      { new T {std::forward<Args> (args)...}
      , Deleter {destroy}
      };
  }
}
