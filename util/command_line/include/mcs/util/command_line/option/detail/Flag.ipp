// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::command_line::option
{
  constexpr Flag::Flag (std::string_view name) noexcept
    : _name {name}
  {}

  constexpr Flag::operator std::string_view() const noexcept
  {
    return _name;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::command_line::option::Flag>::parse
      ( ParseContext& context
      )
  {
    return _formatter.parse (context);
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::util::command_line::option::Flag>::format
      ( mcs::util::command_line::option::Flag const& flag
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return _formatter.format
      ( std::string_view {flag}
      , context
      );
  }
}
