// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>

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
