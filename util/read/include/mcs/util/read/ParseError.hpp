// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <string>

namespace mcs::util::read
{
  struct ParseError : public mcs::Error
  {
  public:
    ~ParseError() override;
    ParseError (ParseError const&) = default;
    ParseError (ParseError&&) noexcept = default;
    auto operator= (ParseError const&) -> ParseError& = default;
    auto operator= (ParseError&&) noexcept  -> ParseError& = default;

  private:
    template<typename> friend struct State;

    explicit ParseError (std::string);
  };
}
