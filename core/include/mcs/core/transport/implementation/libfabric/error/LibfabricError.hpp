// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <string>

namespace mcs::core::transport::implementation::libfabric::error
{
  struct LibfabricError : public mcs::Error
  {
    struct FunctionName
    {
      explicit FunctionName (char const*) noexcept;

    private:
      friend struct LibfabricError;
      std::string _value;
    };

    struct ErrorName
    {
      explicit ErrorName (char const*) noexcept;

    private:
      friend struct LibfabricError;
      std::string _value;
    };

    LibfabricError (FunctionName, ErrorName);
    MCS_ERROR_COPY_MOVE_DEFAULT (LibfabricError);
  };
}
