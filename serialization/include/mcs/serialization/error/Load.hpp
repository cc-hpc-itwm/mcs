// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/Error.hpp>

namespace mcs::serialization::error
{
  // An exception happened when loading a value from an archive.
  //
  struct Load : public mcs::Error
  {
    explicit Load() noexcept;

    MCS_ERROR_COPY_MOVE_DEFAULT (Load);
  };
}
