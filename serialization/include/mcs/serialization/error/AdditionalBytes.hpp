// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/Error.hpp>

namespace mcs::serialization::error
{
  // An load_from<T> (ia) did not consume the complete archive.
  //
  struct AdditionalBytes : public mcs::Error
  {
    explicit AdditionalBytes (std::size_t) noexcept;

    MCS_ERROR_COPY_MOVE_DEFAULT (AdditionalBytes);
  };
}
