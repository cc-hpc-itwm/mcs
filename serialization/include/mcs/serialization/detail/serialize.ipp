// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto operator& (IArchive& ia, T& x) -> IArchive&
  {
    x = load<T> (ia);

    return ia;
  }

  template<is_serializable T>
    constexpr auto operator& (OArchive& oa, T const& x) -> OArchive&
  {
    return save (oa, x);
  }

  template<is_serializable T>
    constexpr auto operator& (OArchive& oa, T& x) -> OArchive&
  {
    return save (oa, x);
  }

  template<is_empty T>
    constexpr auto operator& (OArchive& oa, T&& x) -> OArchive&
  {
    return save (oa, std::forward<T> (x));
  }
}
