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

    ~Save() override;
    Save (Save const&) = default;
    Save (Save&&) noexcept = default;
    auto operator= (Save const&) -> Save& = default;
    auto operator= (Save&&) noexcept  -> Save& = default;
  };
}
