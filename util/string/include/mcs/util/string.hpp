// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <string>
#include <type_traits>

namespace mcs::util
{
  // Wrapper around std::string that takes care of quoting in read and
  // fmt and has the property
  //
  //     read . fmt = identity'
  //
  struct string
  {
    template<typename... Args>
      constexpr explicit string (Args&&...)
        noexcept (std::is_nothrow_constructible_v<std::string, Args&&...>)
      ;

    operator std::string() const;

    auto operator<=> (string const&) const noexcept = default;

  private:
    std::string _str;

    MCS_UTIL_HASH_ACCESS();
  };
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (mcs::util::string);
}

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (string);
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
    ( _str
    , mcs::util::string
    );
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::string
      );
}

#include "detail/string.ipp"
