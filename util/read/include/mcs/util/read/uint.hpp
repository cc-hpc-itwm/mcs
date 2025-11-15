// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>
#include <mcs/util/read/declare.hpp>

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( std::uint_least8_t
    );
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( std::uint_least16_t
    );
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( std::uint_least32_t
    );
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( std::uint_least64_t
    );
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( std::byte
    );
}

#include "detail/uint.ipp"
