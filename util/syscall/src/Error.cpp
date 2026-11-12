// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/syscall/Error.hpp>
#include <utility>

namespace mcs::util::syscall
{
  Error::~Error() = default;
}

namespace mcs::util::syscall
{
  auto syscall_error (int ec) -> std::system_error
  {
    return std::error_code (ec, std::system_category());
  }
}
