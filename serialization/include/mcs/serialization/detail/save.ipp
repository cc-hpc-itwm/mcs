// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/error/Save.hpp>
#include <memory>

namespace mcs::serialization
{
  template<is_serializable T>
    constexpr auto save (OArchive& oa, T const& x) -> OArchive&
  try
  {
    if constexpr (is_empty<T>)
    {
      return oa;
    }

    if constexpr (is_trivial<T>)
    {
      return oa.append (std::span (std::addressof (x), 1));
    }

    if constexpr (has_intrusive_ctor_and_save<T>)
    {
      return x.save (oa);
    }

    if constexpr (has_nonintrusive_Implementation<T>)
    {
      return Implementation<T>::output (oa, x);
    }
  }
  catch (...)
  {
    std::throw_with_nested (error::Save{});
  }
}
