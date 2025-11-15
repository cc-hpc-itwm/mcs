// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <mcs/core/storage/c_api/segment_id.h>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/access.hpp>
#include <mcs/util/read/declare.hpp>

namespace mcs::core::storage::segment
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
    friend struct util::Cast<ID, ::mcs_core_storage_segment_id>;
    friend struct util::Cast<::mcs_core_storage_segment_id, ID>;
  };
}

namespace mcs::util
{
  template<>
    struct Cast<core::storage::segment::ID, ::mcs_core_storage_segment_id>
  {
    constexpr auto operator()
      ( ::mcs_core_storage_segment_id
      ) const -> core::storage::segment::ID
      ;
  };
  template<>
    struct Cast<::mcs_core_storage_segment_id, core::storage::segment::ID>
  {
    constexpr auto operator()
      ( core::storage::segment::ID
      ) const -> ::mcs_core_storage_segment_id
      ;
  };
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (mcs::core::storage::segment::ID);
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
    (mcs::core::storage::segment::ID);
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (core::storage::segment::ID);
}

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (core::storage::segment::ID);
}

#include "detail/ID.ipp"
