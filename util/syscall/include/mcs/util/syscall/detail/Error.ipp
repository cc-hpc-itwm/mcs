// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <type_traits>
#include <utility>

namespace mcs::util::syscall
{
  template<typename T>
    auto negative_one_throws_with_errno (auto rc) -> T
  {
    if (std::cmp_equal (rc, -1))
    {
      throw syscall_error (errno);
    }

    if constexpr (!std::is_void_v<T>)
    {
      static_assert (std::is_same_v<decltype (rc), T>);

      return rc;
    }
  }

  template<typename Pointer>
    auto nullptr_fails_with_errno (Pointer pointer) -> Pointer
  {
    if (pointer == nullptr)
    {
      throw syscall_error (errno);
    }

    return pointer;
  }

  template<typename R>
    auto never_fails (R rc) -> R
  {
    return rc;
  }
}
