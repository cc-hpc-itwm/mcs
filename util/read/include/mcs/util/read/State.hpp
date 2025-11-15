// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <optional>
#include <string>

namespace mcs::util::read
{
  // non-owning parse state
  //
  template<typename Char>
    struct [[nodiscard]] State
  {
    // Returns true iff input is available
    //
    constexpr auto end() const noexcept -> bool;

    // Returns next character of available input.
    //
    // Expects (!end())
    //
    [[nodiscard]] constexpr auto peek() const noexcept -> Char;

    // Returns and consumes next character of available input.
    //
    // Expects (!end())
    //
    [[nodiscard]] constexpr auto head() noexcept -> Char;

    // Ignores next character of available input.
    //
    // Expects (!end())
    //
    constexpr auto skip() noexcept -> void;

    // Produce an error with the state being included into the message.
    //
    template<fmt::formattable Reason>
      constexpr auto error (Reason) const;

  private:
    Char* _input;
    Char* _pos {_input};
    std::optional<std::size_t> _size{};

    [[nodiscard]] constexpr explicit State (Char*) noexcept;
    [[nodiscard]] constexpr explicit State (Char*, std::size_t) noexcept;

    template<typename Ch>
      friend constexpr auto make_state (Ch*) noexcept;
    template<typename Ch>
      friend constexpr auto make_state (Ch*, std::size_t) noexcept;
  };

  template<typename Char>
    [[nodiscard]] constexpr auto make_state (Char*) noexcept;
  template<typename Char>
    [[nodiscard]] constexpr auto make_state (Char*, std::size_t) noexcept;

  template<typename Char>
    [[nodiscard]] auto make_state (std::basic_string<Char> const&) noexcept;
}

#include "detail/State.ipp"
