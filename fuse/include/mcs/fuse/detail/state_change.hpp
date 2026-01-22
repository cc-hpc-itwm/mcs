// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/Error.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <type_traits>
#include <utility>

namespace mcs::fuse::detail
{
  template<typename Cleanup>
    requires (std::invocable<Cleanup>)
    struct StateChange
  {
    template<typename Description, typename Create>
    requires (  std::invocable<Create>
             && std::is_same_v<std::invoke_result_t<Create>, int>
             )
      StateChange
        ( Cleanup cleanup
        , Description description
        , Create&& create
        )
      : _cleanup {std::move (cleanup)}
    {
      if ( auto const ec {std::invoke (std::forward<Create> (create))}
         ; ec != 0
         )
      {
        throw mcs::Error
          { fmt::format ("{} failed with error code {}", description, ec)
          };
      }
    }

    StateChange (StateChange const&) = delete;
    StateChange (StateChange&&) = delete;
    auto operator= (StateChange const&) -> StateChange& = delete;
    auto operator= (StateChange&&) -> StateChange& = delete;

    ~StateChange() noexcept
    try
    {
      std::invoke (_cleanup);
    }
    catch (...)
    {
      util::FMT::print_noexcept
        ( stderr
        , "fuse::state_change::cleanup: {}\n"
        , std::current_exception()
        );
    }

  private:
    Cleanup _cleanup;
  };

  // Scoped state change using a create and a cleanup function.
  //
  template<typename Cleanup, typename Description, typename Create>
    requires (  std::invocable<Create>
             && std::is_same_v<std::invoke_result_t<Create>, int>
             && std::invocable<Cleanup>
             )
  auto state_change
    ( Cleanup cleanup
    , Description description
    , Create&& create
    ) -> StateChange<Cleanup>
  {
    return StateChange<Cleanup>
      { std::move (cleanup)
      , std::move (description)
      , std::forward<Create> (create)
      };
  }
}
