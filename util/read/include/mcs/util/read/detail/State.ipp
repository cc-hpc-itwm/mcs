// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <iterator>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/ParseError.hpp>
#include <string>

namespace mcs::util::read
{
  template<typename Char>
    constexpr State<Char>::State (Char* input) noexcept
      : _input {input}
  {}
  template<typename Char>
    constexpr State<Char>::State (Char* input, std::size_t size) noexcept
      : _input {input}
      , _size {size}
  {}

  template<typename Char>
    constexpr auto State<Char>::end() const noexcept -> bool
  {
    if (!_pos)
    {
      return true;
    }

    if (_size)
    {
      return *_size == 0;
    }

    return peek() == '\0';
  }

  template<typename Char>
    constexpr auto State<Char>::peek() const noexcept -> Char
  {
    return *_pos;
  }

  template<typename Char>
    constexpr auto State<Char>::head() noexcept -> Char
  {
    auto c {*_pos};
    skip();
    return c;
  }

  template<typename Char>
    constexpr auto State<Char>::skip() noexcept -> void
  {
    ++_pos;

    if (_size.has_value())
    {
      --*_size;
    }
  }

  template<typename Char>
    template<fmt::formattable Reason>
      constexpr auto State<Char>::error (Reason reason) const
  {
    return ParseError
      { fmt::format
          ( "ParseError:\n{0}\n{1}^\n{1}: {2}"
          , _size.has_value()
            ? std::string {_input, _pos + _size.value()}
            : std::string {_input}
          , std::string
              (util::cast<std::size_t> (std::distance (_input, _pos)), ' ')
          , reason
          )
      };
  }
}

namespace mcs::util::read
{
  template<typename Char>
    constexpr auto make_state (Char* input) noexcept
  {
    return State<Char> {input};
  }

  template<typename Char>
    constexpr auto make_state (Char* input, std::size_t size) noexcept
  {
    return State<Char> {input, size};
  }

  template<typename Char>
    auto make_state (std::basic_string<Char> const& s) noexcept
  {
    return make_state (s.c_str(), s.size());
  }
}
