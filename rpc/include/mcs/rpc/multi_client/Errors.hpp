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

    ~Errors() override;
    Errors (Errors const&) = default;
    Errors (Errors&&) noexcept = default;
    auto operator= (Errors const&) -> Errors& = default;
    auto operator= (Errors&&) noexcept  -> Errors& = default;

  private:
    std::list<std::exception_ptr> _errors;
  };
}
