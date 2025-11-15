// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/rpc/Concepts.hpp>
#include <utility>

namespace mcs::rpc
{
  template<typename Client, is_command Command>
    struct SyncRemoteFunction
  {
    constexpr explicit SyncRemoteFunction (Client&) noexcept;

    constexpr auto operator()
      ( std::reference_wrapper<Command const>
      ) -> typename Command::Response;

    constexpr auto operator()
      ( Command&&
      ) -> typename Command::Response;

    template<typename... CommandArgs>
      constexpr auto operator()
        ( CommandArgs&&...
        ) -> typename Command::Response;

  private:
    template<is_command Command_, typename Client_>
      friend constexpr auto make_sync_remote_function
        ( Client_&
        ) noexcept;

    Client& _client;
  };
}

#include "detail/SyncRemoteFunction.ipp"
