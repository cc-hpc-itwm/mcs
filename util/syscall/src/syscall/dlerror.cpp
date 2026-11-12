// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <mcs/Error.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dlclose.hpp>
#include <mcs/util/syscall/dlopen.hpp>
#include <mcs/util/syscall/dlsym.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <mcs/util/syscall/raise.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace mcs::util::syscall::error
{
  DLError::DLError (std::string call, char const* reason)
    : mcs::Error {fmt::format ("dlerror ({}): {}", call, reason)}
    , _call {call}
    , _reason {reason}
  {}

  DLError::~DLError() = default;

  auto DLError::call() const noexcept -> std::string
  {
    return _call;
  }

  auto DLError::reason() const noexcept -> std::string
  {
    return _reason;
  }
}

namespace mcs::util::syscall::detail
{
  CheckDLError::CheckDLError (std::string call)
    : _call {std::move (call)}
  {
    throw_if_there_is_a_dlerror();
  }

  auto CheckDLError::throw_if_there_is_a_dlerror() const -> void
  {
    if (auto const reason {::dlerror()})
    {
      throw mcs::util::syscall::error::DLError {_call, reason};
    }
  }

  std::mutex CheckDLError::_guard;
}
