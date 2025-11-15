// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/ResultOrError.hpp>
#include <mcs/serialization/OArchive.hpp>

namespace mcs::rpc::detail
{
  struct ResultHolder
  {
    // \note default ctor required by asio::co_spawn
    //
    ResultHolder() = default;

    template<typename Response>
      constexpr explicit ResultHolder
        ( CallID
        , ResultOrError<Response>
        );

    ~ResultHolder() noexcept = default;
    ResultHolder (ResultHolder const&) = delete;
    ResultHolder (ResultHolder&&) noexcept = default;
    auto operator= (ResultHolder const&) -> ResultHolder& = delete;
    auto operator= (ResultHolder&&) noexcept -> ResultHolder& = default;

    std::function<serialization::OArchive()> archive;
  };
}

#include "detail/ResultHolder.ipp"
