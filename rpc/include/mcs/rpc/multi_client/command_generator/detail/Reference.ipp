// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::rpc::multi_client::command_generator
{
  template<typename T>
    constexpr Reference<T>::Reference
      ( std::reference_wrapper<T const> ref
      ) noexcept
        : _ref {ref}
  {}

  template<typename T>
    template<typename Client>
      constexpr auto Reference<T>::operator()
        ( Client const&
        ) const noexcept -> std::reference_wrapper<T const>
  {
    return _ref;
  }
}
