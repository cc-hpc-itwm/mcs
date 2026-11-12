// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/Error.hpp>
#include <utility>

namespace mcs
{
  Error::Error (std::string what)
    : _what {std::move (what)}
  {}

  auto Error::what() const noexcept -> char const*
  {
    return _what.c_str();
  }
}
