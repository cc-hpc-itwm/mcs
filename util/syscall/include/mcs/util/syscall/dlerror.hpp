// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <string>

namespace
{
  struct CheckDLError;
}

namespace mcs::util::syscall::error
{
  // Functions that check dlerror() throw this exception in case there
  // is an error detected.
  //
  struct DLError : public mcs::Error
  {
    [[nodiscard]] auto call() const noexcept -> std::string;
    [[nodiscard]] auto reason() const noexcept -> std::string;

    MCS_ERROR_COPY_MOVE_DEFAULT (DLError);

  private:
    friend struct ::CheckDLError;

    DLError (std::string, char const*);

    std::string _call;
    std::string _reason;
  };
}
