// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <system_error>

namespace mcs::util::syscall
{
  // If a syscall fails, then its parameters are captured in the
  // message of this error.
  //
  struct Error : public mcs::Error
  {
    using mcs::Error::Error;
    ~Error() override;
    Error (Error const&) = default;
    Error (Error&&) noexcept = default;
    auto operator= (Error const&) -> Error& = default;
    auto operator= (Error&&) noexcept  -> Error& = default;
  };
}

namespace mcs::util::syscall
{
  // Create a std::system_error with the given error code.
  //
  auto syscall_error (int) -> std::system_error;

  // If rc is -1, then throw with errno as error code.
  //
  template<typename T>
    auto negative_one_throws_with_errno (auto rc) -> T;

  // If pointer is the nullptr, then throw with errno as error code
  //
  template<typename Pointer>
    auto nullptr_fails_with_errno (Pointer pointer) -> Pointer;

  // Some functions never fail and return a code.
  //
  template<typename R>
    auto never_fails (R) -> R;
}

#include "detail/Error.ipp"
