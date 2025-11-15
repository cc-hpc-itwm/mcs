// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/serialization/error/Load.hpp>
#include <memory>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto load (IArchive& ia) -> T
  try
  {
    if constexpr (is_empty<T>)
    {
      return T{};
    }
    else if constexpr (is_trivial<T>)
    {
      auto x {T{}};

      ia.extract (std::addressof (x), sizeof (x));

      return x;
    }
    else if constexpr (has_intrusive_ctor_and_save<T>)
    {
      return T {ia};
    }
    else if constexpr (has_intrusive_serialize<T>)
    {
      auto x {T{}};

      x.serialize (ia, 0u);

      return x;
    }
    else if constexpr (has_nonintrusive_Implementation<T>)
    {
      return Implementation<T>::input (ia);
    }
  }
  catch (...)
  {
    std::throw_with_nested (error::Load{});
  }
}
