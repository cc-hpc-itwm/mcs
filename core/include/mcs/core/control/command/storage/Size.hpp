// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::core::control::command::storage
{
  struct Size
  {
    struct Response
    {
      [[nodiscard]] constexpr Response
        ( core::storage::MaxSize
        , core::memory::Size
        ) noexcept
        ;
      [[nodiscard]] constexpr auto max
        (
        ) const noexcept -> core::storage::MaxSize
        ;
      [[nodiscard]] constexpr auto used
        (
        ) const noexcept -> core::memory::Size
        ;

    private:
      core::storage::MaxSize _max;
      core::memory::Size _used;

      MCS_UTIL_TUPLISH_ACCESS();
    };

    core::storage::ID storage_id;
    core::storage::Parameter storage_parameter_size_max;
    core::storage::Parameter storage_parameter_size_used;
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::storage::Size::Response);

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  (mcs::core::control::command::storage::Size);

#include "detail/Size.ipp"
