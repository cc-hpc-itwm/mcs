// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/Error.hpp>

namespace mcs
{
  Error::Error (std::string what)
    : _what {what}
  {}

  auto Error::what() const noexcept -> char const*
  {
    return _what.c_str();
  }
}
