// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <iov/meta.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::iov_backend::storage
{
  struct ID
  {
    [[nodiscard]] constexpr ID() noexcept = default;

    constexpr auto operator++() noexcept -> ID&;

    [[nodiscard]] constexpr auto operator<=>
      ( ID const&
      ) const noexcept = default
      ;

  private:
    using underlying_type = std::uintmax_t;

    underlying_type _value {0u};

    [[nodiscard]] constexpr ID (underlying_type) noexcept;

    MCS_UTIL_FMT_ACCESS();
    MCS_UTIL_HASH_ACCESS();
    MCS_SERIALIZATION_ACCESS();
    MCS_UTIL_READ_ACCESS();
  };
}


MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::iov_backend::storage::ID);

namespace std
{
  template<>
    MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
      ( mcs::iov_backend::storage::ID
      );
}

#include "detail/ID.ipp"
