// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::iov_backend
{
  struct UsedStorage
  {
    core::memory::Range _range;
    core::storage::segment::ID _segment_id;
    storage::ID _storage_id;

    constexpr auto operator<=> (UsedStorage const&) const noexcept = default;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::iov_backend::UsedStorage);

#include "detail/UsedStorage.ipp"
