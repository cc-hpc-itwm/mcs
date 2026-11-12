// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::util::command_line::option
{
  constexpr Parameter::Parameter
    ( std::string_view name
    , Presence::Policy presence
    , Multiplicity::Policy multiplicity
    ) noexcept
    : _name {name}
    , _presence {std::move (presence)}
    , _multiplicity {std::move (multiplicity)}
  {}

  constexpr Parameter::operator std::string_view() const noexcept
  {
    return _name;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::command_line::option::Parameter>::parse
      ( ParseContext& context
      )
  {
    return _formatter.parse (context);
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::util::command_line::option::Parameter>::format
      ( mcs::util::command_line::option::Parameter const& parameter
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return _formatter.format
      ( std::string_view {parameter}
      , context
      );
  }
}
