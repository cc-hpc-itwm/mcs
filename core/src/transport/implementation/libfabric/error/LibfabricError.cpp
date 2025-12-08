// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/Error.hpp>
#include <mcs/core/transport/implementation/libfabric/error/LibfabricError.hpp>

namespace mcs::core::transport::implementation::libfabric::error
{
  LibfabricError::FunctionName::FunctionName
    ( char const* value
    ) noexcept
      : _value {value}
  {}

  LibfabricError::ErrorName::ErrorName
    ( char const* value
    ) noexcept
      : _value {value}
  {}

  LibfabricError::LibfabricError
    ( FunctionName function_name
    , ErrorName error_name
    )
      : mcs::Error
        { fmt::format
          ( "mcs::core::transport::implementation::libfabric::"
          "error::LibfabricError: {}: {}"
          , function_name._value
          , error_name._value
          )
        }
  {}
  LibfabricError::~LibfabricError() = default;
}
