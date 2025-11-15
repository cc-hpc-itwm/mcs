// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/error/internal/UnknownCommand.hpp>

namespace mcs::rpc::error::internal
{
  UnknownCommand::UnknownCommand
    () noexcept
      : mcs::Error {"Unknown command."}
  {}
  UnknownCommand::~UnknownCommand() = default;
}
