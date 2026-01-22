// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <exception>
#include <string>
#include <type_traits>

namespace mcs
{
  // Base for exceptions.
  //
  // EXAMPLE:
  //
  // class C
  // {
  //   struct Error : public mcs::Error
  //   {
  //     Error();
  //     ~Error() override;
  //     Error (Error const&) = default;
  //     Error (Error&&) noexcept = default;
  //     auto operator= (Error const&) -> Error& = default;
  //     auto operator= (Error&&) noexcept  -> Error& = default;
  //   };
  // };
  //
  // C::Error::Error()
  //   : mcs::Error {"C::Error"}
  // {}
  // C::Error::~Error() = default;
  //
  class Error : public std::exception
  {
  public:
    [[nodiscard]] explicit Error (std::string);

    [[nodiscard]] auto what() const noexcept -> char const* final;
    ~Error() noexcept override = default;

    Error (Error const&) = default;
    Error (Error&&) = default;
    auto operator= (Error const&) -> Error& = default;
    auto operator= (Error&&) -> Error& = default;

  private:
    std::string _what;
  };
}
