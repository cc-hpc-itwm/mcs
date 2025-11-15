// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/declare.hpp>
#include <string>
#include <variant>

namespace mcs::rpc::detail
{
  template<typename T>
    struct Result
  {
    T value;

    explicit Result (decltype (value));
    MCS_SERIALIZATION_DECLARE_INTRUSIVE_CTOR_AND_SAVE (Result);
  };

  template<>
    struct Result<void>
  {
  };

  struct Error
  {
    std::string reason;

    explicit Error (decltype (reason));
    MCS_SERIALIZATION_DECLARE_INTRUSIVE_CTOR_AND_SAVE (Error);
  };

  template<typename T>
    using ResultOrError = std::variant<Error, Result<T>>;

  template<typename T> auto make_result (T) -> ResultOrError<T>;
  auto make_result() -> ResultOrError<void>;

  template<typename T> auto make_error (std::string) -> ResultOrError<T>;
}

#include "detail/ResultOrError.ipp"
