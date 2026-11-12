// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>

namespace mcs::util::command_line::error
{
  struct UnknownMarker : public mcs::Error
  {
    using mcs::Error::Error;
  };
}
