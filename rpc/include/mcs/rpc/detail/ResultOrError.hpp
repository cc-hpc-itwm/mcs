// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <string>
#include <variant>

namespace mcs::rpc::detail
{
  template<typename T>
    struct Result
  {
    T value;

    explicit Result (decltype (value));
    explicit Result (serialization::IArchive&);
    auto save (serialization::OArchive&) const -> serialization::OArchive&;
  };

  template<>
    struct Result<void>
  {
  };

  struct Error
  {
    std::string reason;

    explicit Error (decltype (reason));
    explicit Error (serialization::IArchive&);
    auto save (serialization::OArchive&) const -> serialization::OArchive&;
  };

  template<typename T>
    using ResultOrError = std::variant<Error, Result<T>>;

  template<typename T> auto make_result (T) -> ResultOrError<T>;
  auto make_result() -> ResultOrError<void>;

  template<typename T> auto make_error (std::string) -> ResultOrError<T>;
}

#include "detail/ResultOrError.ipp"
