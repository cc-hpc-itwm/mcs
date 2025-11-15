// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <mcs/Error.hpp>

namespace mcs::serialization::error
{
  // An exception happened when saving a value to an archive.
  //
  struct Save : public mcs::Error
  {
    explicit Save() noexcept;

    MCS_ERROR_COPY_MOVE_DEFAULT (Save);
  };
}
