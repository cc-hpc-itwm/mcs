// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/Error.hpp>

namespace mcs::serialization::error
{
  // AN archive does not contain enough bytes.
  //
  struct NotEnoughBytes : public mcs::Error
  {
    explicit NotEnoughBytes (std::size_t, std::size_t) noexcept;

    [[nodiscard]] constexpr auto wanted() const noexcept -> std::size_t;
    [[nodiscard]] constexpr auto provided() const noexcept -> std::size_t;

    ~NotEnoughBytes() override;
    NotEnoughBytes (NotEnoughBytes const&) = default;
    NotEnoughBytes (NotEnoughBytes&&) noexcept = default;
    auto operator= (NotEnoughBytes const&) -> NotEnoughBytes& = default;
    auto operator= (NotEnoughBytes&&) noexcept  -> NotEnoughBytes& = default;

  private:
    std::size_t _wanted;
    std::size_t _provided;
  };
}

#include "detail/NotEnoughBytes.ipp"
