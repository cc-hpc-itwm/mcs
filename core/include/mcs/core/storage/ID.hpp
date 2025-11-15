// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/access.hpp>
#include <mcs/util/read/declare.hpp>

namespace mcs::core::storage
{
  struct ID
  {
    [[nodiscard]] constexpr ID() noexcept = default;

    constexpr auto operator++() noexcept -> ID&;
    constexpr auto operator<=> (ID const&) const noexcept = default;

  private:
    using underlying_type = std::uintmax_t;

    underlying_type _value {0u};

    [[nodiscard]] constexpr explicit ID (underlying_type) noexcept;

    MCS_UTIL_FMT_ACCESS();
    MCS_UTIL_HASH_ACCESS();
    MCS_SERIALIZATION_ACCESS();
    MCS_UTIL_READ_ACCESS();
  };
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (mcs::core::storage::ID);
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
    ( mcs::core::storage::ID
    );
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (core::storage::ID);
}

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( core::storage::ID
    );
}

#include "detail/ID.ipp"
