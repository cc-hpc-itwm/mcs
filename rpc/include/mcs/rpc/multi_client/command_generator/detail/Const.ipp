// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc::multi_client::command_generator
{
  template<typename T>
    template<typename... Args>
      requires (std::is_constructible_v<T, Args...>)
      constexpr Const<T>::Const
        ( Args&&... args
        ) noexcept (std::is_nothrow_constructible_v<T, Args...>)
          : _value {std::forward<Args> (args)...}
  {}

  template<typename T>
    template<typename Client>
      constexpr auto Const<T>::operator()
        ( Client const& client
        ) const noexcept -> std::reference_wrapper<T const>
  {
    return _ref (client);
  }
}
