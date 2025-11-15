// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/fwd_capture.hpp>
#include <type_traits>
#include <utility>

namespace mcs::nonstd::detail
{
  // NOLINTBEGIN (cppcoreguidelines-missing-std-forward)
  template<typename Description, typename Fun>
    requires (std::is_invocable_v<Fun>)
    auto execute_and_die_on_exception_function
    ( Description&& description
    , Fun&& fun
    ) noexcept
  // NOLINTEND (cppcoreguidelines-missing-std-forward)
  {
    return [ fun = util::fwd_capture (std::forward<Fun> (fun))
           , description =
               util::fwd_capture (std::forward<Description> (description))
           ]() noexcept
      {
        return util::execute_and_die_on_exception
          ( std::move (description).get()
          , std::move (fun).get()
          );
      };
  }
}
