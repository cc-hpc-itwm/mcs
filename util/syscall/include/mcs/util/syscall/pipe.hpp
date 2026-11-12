// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <array>

namespace mcs::util::syscall
{
  struct Pipe
  {
    [[nodiscard]] constexpr auto read() const noexcept -> int;
    [[nodiscard]] constexpr auto write() const noexcept -> int;

  private:
    friend auto pipe() -> Pipe;

    [[nodiscard]] constexpr explicit Pipe (std::array<int, 2>) noexcept;

    std::array<int, 2> _pipefd;
  };

  [[nodiscard]] auto pipe() -> Pipe;
}

#include "detail/pipe.ipp"
