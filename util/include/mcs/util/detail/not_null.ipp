// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util
{
  template<typename T>
    constexpr not_null<T>::not_null (T* pointer)
      : _pointer {pointer}
  {
    if (!pointer)
    {
      throw typename Error::MustNotBeNull{};
    }
  }

  template<typename T>
    [[nodiscard]] constexpr auto not_null<T>::operator->() const noexcept -> T*
  {
    return _pointer;
  }
  template<typename T>
    [[nodiscard]] constexpr auto not_null<T>::get() const noexcept -> T*
  {
    return _pointer;
  }

  template<typename T>
    [[nodiscard]] constexpr auto not_null<T>::operator*
      (
      ) const noexcept -> T const&
  {
    return *_pointer;
  }

  template<typename T>
    not_null<T>::Error::MustNotBeNull::MustNotBeNull()
      : mcs::Error {"not_null: must not be null"}
  {}
  template<typename T>
    not_null<T>::Error::MustNotBeNull::~MustNotBeNull() = default;
}
