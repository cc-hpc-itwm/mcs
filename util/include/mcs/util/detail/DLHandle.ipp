// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/util/syscall/dlsym.hpp>

namespace mcs::util
{
  template<typename T>
    auto DLHandle::symbol (char const* name) const -> T*
  try
  {
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconditionally-supported"
#endif
    return reinterpret_cast<T*>
      ( syscall::dlsym (_loaded_library.get(), name)
      );
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
  }
  catch (...)
  {
    std::throw_with_nested (Error::Symbol {name});
  }
}

namespace mcs::util
{
  constexpr auto DLHandle::Error::Symbol::name() const noexcept -> char const*
  {
    return _name;
  }
}
