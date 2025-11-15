// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <string>

namespace mcs::rpc::error
{
  // The remote handler has thrown an exception during command
  // handling and the _message of that exception_ is transported to
  // the client, wrapped into an HandlerExcpetion.
  //
  // \todo transport the complete (de/serialized!) exception
  //
  struct HandlerError : public mcs::Error
  {
    explicit HandlerError (std::string) noexcept;

    MCS_ERROR_COPY_MOVE_DEFAULT (HandlerError);
  };
}
