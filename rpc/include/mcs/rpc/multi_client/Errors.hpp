// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <list>
#include <mcs/Error.hpp>

namespace mcs::rpc::multi_client
{
  struct Errors : public mcs::Error
  {
    explicit Errors (std::list<std::exception_ptr>) noexcept;

    auto errors() const noexcept -> std::list<std::exception_ptr> const&;

    MCS_ERROR_COPY_MOVE_DEFAULT (Errors);

  private:
    std::list<std::exception_ptr> _errors;
  };
}
