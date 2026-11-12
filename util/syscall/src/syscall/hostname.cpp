// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <climits>
#include <exception>
#include <functional>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <unistd.h>
#include <vector>

namespace mcs::util::syscall
{
  auto hostname() -> char const*
  try
  {
    static auto buffer {std::vector<char> (HOST_NAME_MAX + 1)};
    static auto name
      { std::invoke
        ( []
          {
            buffer.back() = '\0';

            negative_one_throws_with_errno<void>
              (::gethostname (buffer.data(), HOST_NAME_MAX));

            return buffer.data();
          }
        )
      };

    return name;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error {"syscall::hostname()"}
      );
  }
}
