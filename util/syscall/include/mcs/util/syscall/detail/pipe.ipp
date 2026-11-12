// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::syscall
{
  constexpr Pipe::Pipe (std::array<int, 2> pipefd) noexcept
    : _pipefd {pipefd}
  {}

  constexpr auto Pipe::read() const noexcept -> int
  {
    return std::get<0> (_pipefd);
  }
  constexpr auto Pipe::write() const noexcept -> int
  {
    return std::get<1> (_pipefd);
  }
}
