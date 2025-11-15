// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/nonstd/scope.hpp>
#include <utility>

namespace mcs::rpc::multi_client::command_generator
{
  template<typename R>
    constexpr Range<R>::Range
      ( R range
      ) noexcept
        : _range {std::move (range)}
  {}

  template<typename R>
    template<typename Client>
      constexpr auto Range<R>::operator()
        ( Client const&
        ) noexcept
  {
    auto const increment
      { nonstd::make_scope_success ([&]() noexcept { ++_value; })
      };

    return *_value;
  }

  template<typename R>
    constexpr auto make_range (R range) noexcept
  {
    return Range<R> {std::move (range)};
  }
}
