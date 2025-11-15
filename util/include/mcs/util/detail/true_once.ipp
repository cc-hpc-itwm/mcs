// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util
{
  constexpr TrueOnce::operator bool() const noexcept
  {
    auto v {_value};
    _value = false;
    return v;
  }
}
