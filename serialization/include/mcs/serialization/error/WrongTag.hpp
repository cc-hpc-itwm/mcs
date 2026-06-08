// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/Error.hpp>

namespace mcs::serialization::error
{
  // The wrong archive tag was requested
  //
  struct WrongTag : public mcs::Error
  {
    explicit WrongTag() noexcept;

    ~WrongTag() override;
    WrongTag (WrongTag const&) = default;
    WrongTag (WrongTag&&) noexcept = default;
    auto operator= (WrongTag const&) -> WrongTag& = default;
    auto operator= (WrongTag&&) noexcept  -> WrongTag& = default;
  };
}
