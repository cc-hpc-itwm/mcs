// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/rpc/error/Completion.hpp>
#include <mcs/util/FMT/STD/exception.hpp>

namespace mcs::rpc::error
{
  Completion::Completion
    ( std::exception_ptr reason
    ) noexcept
      : mcs::Error {fmt::format ("rpc::error::Completion: {}", reason)}
      , _reason {reason}
  {}
  auto Completion::reason() const noexcept -> std::exception_ptr
  {
    return _reason;
  }
  Completion::~Completion() = default;
}
