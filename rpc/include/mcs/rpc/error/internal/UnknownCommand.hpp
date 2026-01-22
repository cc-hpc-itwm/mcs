// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>

namespace mcs::rpc::error::internal
{
  struct UnknownCommand : public mcs::Error
  {
    UnknownCommand() noexcept;

    ~UnknownCommand() override;
    UnknownCommand (UnknownCommand const&) = default;
    UnknownCommand (UnknownCommand&&) noexcept = default;
    auto operator= (UnknownCommand const&) -> UnknownCommand& = default;
    auto operator= (UnknownCommand&&) noexcept  -> UnknownCommand& = default;
  };
}
