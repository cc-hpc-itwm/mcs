// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/rpc/error/HandlerException.hpp>

namespace mcs::rpc::error
{
  HandlerError::HandlerError
    ( std::string reason
    ) noexcept
      : mcs::Error {fmt::format ("rpc::error::HandlerError: {}", reason)}
  {}
  HandlerError::~HandlerError() = default;
}
