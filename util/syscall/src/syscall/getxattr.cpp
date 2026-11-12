// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/getxattr.hpp>

namespace mcs::util::syscall
{
  auto getxattr ( char const* path
                , char const* name
                , void* value
                , size_t size
                ) -> ssize_t
  {
    return negative_one_throws_with_errno<ssize_t>
      ( ::getxattr (path, name, value, size)
      );
  }
}
