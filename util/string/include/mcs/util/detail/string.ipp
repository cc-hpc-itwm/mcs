// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/Symbol.hpp>
#include <mcs/util/read/skip_whitespace.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::util
{
  template<typename... Args>
    constexpr string::string (Args&&... args)
      noexcept (std::is_nothrow_constructible_v<std::string, Args&&...>)
        : _str {std::forward<Args> (args)...}
  {}
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::string>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::string>::format
      ( mcs::util::string const& str
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    fmt::format_to (ctx.out(), "\"");

    for (auto c : static_cast<std::string> (str))
    {
      if (c == '"')
      {
        fmt::format_to (ctx.out(), "{}", c);
      }

      fmt::format_to (ctx.out(), "{}", c);
    }

    return fmt::format_to (ctx.out(), "\"");
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<string>::read
      ( State<Char>& state
      ) -> string
  {
    skip_whitespace (state);

    auto const q {'"'};

    symbol (std::string (1, q)) (state);

    auto taken {std::string{}};

    while (!state.end())
    {
      if (state.peek() != q)
      {
        taken += state.head();
      }
      else
      {
        state.skip();

        if (state.end() || state.peek() != q)
        {
          return string {taken};
        }
        else
        {
          taken += state.head();
        }
      }
    }

    throw std::runtime_error {fmt::format ("characters_with_{}_doubled", q)};
  }
}
