// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/util/syscall/dlsym.hpp>

namespace mcs::util
{
  template<typename T>
    auto DLHandle::symbol (char const* name) const -> T*
  try
  {
    union
    {
      void* _ptr;
      T* _data;
    } symbol {nullptr};

    symbol._ptr = syscall::dlsym (_loaded_library.get(), name);

    return symbol._data;
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
