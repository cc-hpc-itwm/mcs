// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/format.h>
#include <string_view>
#include <variant>

namespace mcs::util::command_line
{
  struct Parsed;
}

namespace mcs::util::command_line::option
{
  // Descriptors borrow their names. Name characters must remain alive
  // and unchanged while a descriptor or a schema containing it is
  // used.
  //
  struct Parameter
  {
    struct Presence
    {
      struct Optional{};
      struct Required{};

      using Policy = std::variant<Optional, Required>;
    };

    struct Multiplicity
    {
      struct Unique{};
      struct Repeatable{};

      using Policy = std::variant<Unique, Repeatable>;
    };

    explicit constexpr Parameter
      ( std::string_view
      , Presence::Policy = Presence::Required{}
      , Multiplicity::Policy = Multiplicity::Unique{}
      ) noexcept;

    [[nodiscard]] constexpr operator std::string_view() const noexcept;

  private:
    std::string_view _name;
    Presence::Policy _presence;
    Multiplicity::Policy _multiplicity;

    friend struct command_line::Parsed;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::util::command_line::option::Parameter>
  {
    template<typename ParseContext>
      [[nodiscard]] constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      [[nodiscard]] constexpr auto format
        ( mcs::util::command_line::option::Parameter const&
        , FormatContext& context
        ) const -> decltype (context.out());

  private:
    formatter<std::string_view> _formatter{};
  };
}

#include "detail/Parameter.ipp"
