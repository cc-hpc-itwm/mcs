// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/rpc/multi_client/Errors.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <utility>

namespace mcs::rpc::multi_client
{
  Errors::Errors (std::list<std::exception_ptr> errors) noexcept
    : mcs::Error {fmt::format ("mcs::rpc::multi_client::Errors: {}", errors)}
    , _errors  {errors}
  {}
  Errors::~Errors() = default;

  auto Errors::errors() const noexcept -> std::list<std::exception_ptr> const&
  {
    return _errors;
  }
}
