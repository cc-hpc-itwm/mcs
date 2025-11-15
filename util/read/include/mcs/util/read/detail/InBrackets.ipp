// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::read
{
  template<typename Open, typename Inner, typename Close>
    constexpr InBrackets<Open, Inner, Close>::InBrackets
      ( Open open
      , Inner inner
      , Close close
      ) noexcept
        : _open {open}
        , _inner {inner}
        , _close {close}
    {}

  template<typename Open, typename Inner, typename Close>
    template<typename Char>
      constexpr auto InBrackets<Open, Inner, Close>::operator()
        ( State<Char>& state
        ) const
    {
      _open (state);
      auto inner {_inner (state)};
      _close (state);
      return inner;
    }

  template<typename Open, typename Inner, typename Close>
    constexpr auto in_brackets (Open open, Inner inner, Close close) noexcept
  {
    return InBrackets<Open, Inner, Close> {open, inner, close};
  }
}
