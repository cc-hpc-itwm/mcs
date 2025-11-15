// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util::read
{
  template<typename Open, typename Inner, typename Close>
    struct InBrackets
  {
    constexpr explicit InBrackets (Open, Inner, Close) noexcept;

    template<typename Char>
      constexpr auto operator() (State<Char>&) const;

  private:
    Open _open;
    Inner _inner;
    Close _close;
  };

  template<typename Open, typename Inner, typename Close>
    constexpr auto in_brackets (Open, Inner, Close) noexcept;
}

#include "detail/InBrackets.ipp"
