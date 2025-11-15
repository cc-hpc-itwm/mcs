// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE


namespace mcs::rpc::multi_client::command_generator
{
  template<typename T>
    constexpr Sequence<T>::Sequence
      ( T value
      ) noexcept (std::is_nothrow_move_constructible_v<T>)
        : _value {value}
  {}

  template<typename T>
    template<typename Client>
      constexpr auto Sequence<T>::operator()
        ( Client const&
        ) noexcept -> T
  {
    return _value++;
  }
}
