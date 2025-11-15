// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/c_api/chunk_state.h>
#include <mcs/core/storage/c_api/storage.h>
#include <mcs/core/storage/c_api/types.h>

namespace mcs::core::storage::c_api
{
  template<typename> struct Traits;

  template<> struct Traits<chunk::access::Const>
  {
    using ChunkState = ::mcs_core_storage_chunk_const_state;
    using Byte = ::MCS_CORE_STORAGE_BYTE const;
    static constexpr auto chunk_description (::mcs_core_storage const&);
    static constexpr auto chunk_state (::mcs_core_storage const&);
    static constexpr auto chunk_state_destruct (::mcs_core_storage const&);
  };
  template<> struct Traits<chunk::access::Mutable>
  {
    using ChunkState = ::mcs_core_storage_chunk_mutable_state;
    using Byte = ::MCS_CORE_STORAGE_BYTE;
    static constexpr auto chunk_description (::mcs_core_storage const&);
    static constexpr auto chunk_state (::mcs_core_storage const&);
    static constexpr auto chunk_state_destruct (::mcs_core_storage const&);
  };
}

#include "detail/Traits.ipp"
