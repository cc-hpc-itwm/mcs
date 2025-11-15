// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/serialization/detail/IArchive.fwd.hpp>
#include <mcs/serialization/detail/Implementation.fwd.hpp>
#include <mcs/serialization/detail/OArchive.fwd.hpp>
#include <type_traits>

namespace mcs::serialization
{
  template<typename T>
    concept is_empty =
         std::is_default_constructible_v<T>
      && std::is_empty_v<T>
      ;
  template<typename T>
    concept is_trivial =
         std::is_default_constructible_v<T>
      && std::is_trivially_copyable_v<T>
      ;
  template<typename T>
    concept has_intrusive_ctor_and_save =
         std::is_constructible_v<T, IArchive&>
      && requires (T const& x, OArchive& oa)
         {
           { x.save (oa) } -> std::convertible_to<OArchive&>;
         }
      ;
  template<typename T>
    concept has_intrusive_serialize =
         std::is_default_constructible_v<T>
      && requires (T& x, IArchive& ia, OArchive& oa, unsigned int version)
         {
           { x.serialize (ia, version) } -> std::convertible_to<void>;
           { x.serialize (oa, version) } -> std::convertible_to<void>;
         }
      ;
  template<typename T>
    concept has_nonintrusive_Implementation =
       requires (T& x, IArchive& ia, OArchive& oa)
       {
         { Implementation<T>::input (ia) } -> std::convertible_to<T>;
         { Implementation<T>::output (oa, x) } -> std::convertible_to<OArchive&>;
       }
      ;
  template<typename T>
    concept is_serializable =
      (  is_empty<T>
      || is_trivial<T>
      || has_intrusive_ctor_and_save<T>
      || has_intrusive_serialize<T>
      || has_nonintrusive_Implementation<T>
      )
      ;
}
