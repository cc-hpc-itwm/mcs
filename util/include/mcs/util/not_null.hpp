// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>

namespace mcs::util
{
  // A wrapper around a pointer that ensures the pointer is not
  // null. Meant to communicate that access to an object is granted to
  // a function or object without transferring ownership.
  //
  // EXAMPLE:
  //   auto x {0};
  //   auto nn {not_null {std::addressof (x)}};
  //   ASSERT_EQ (*x, 0);
  //
  template<typename T>
    struct not_null
  {
    // Pre: !!x
    //
    constexpr not_null (T* x);

    [[nodiscard]] constexpr auto operator->() const noexcept -> T*;
    [[nodiscard]] constexpr auto operator*() const noexcept -> T const&;

    [[nodiscard]] constexpr auto get() const noexcept -> T*;

    not_null() = delete;
    not_null (not_null const&) = default;
    not_null (not_null&&) = default;
    auto operator= (not_null const&) -> not_null& = default;
    auto operator= (not_null&&) -> not_null& = default;
    ~not_null() = default;

    struct Error
    {
      struct MustNotBeNull : public mcs::Error
      {
        MustNotBeNull();
        MCS_ERROR_COPY_MOVE_DEFAULT (MustNotBeNull);
      };
    };

  private:
    T* _pointer;
  };
}

#include "detail/not_null.ipp"
