// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/DLHandle.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/syscall/dlclose.hpp>
#include <mcs/util/syscall/dlopen.hpp>

namespace mcs::util
{
  DLHandle::DLHandle (std::filesystem::path const& path, int flags)
    : _loaded_library {syscall::dlopen (path.c_str(), flags)}
  {}

  auto DLHandle::DLClose::operator() (void* dlhandle) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "DLClose"
      , syscall::dlclose
      , dlhandle
      );
  }
}

namespace mcs::util
{
  DLHandle::Error::Symbol::Symbol (char const* name)
    : mcs::Error {fmt::format ("DLHandle::symbol ({})", name)}
    , _name {name}
  {}

  DLHandle::Error::Symbol::~Symbol() = default;
}
