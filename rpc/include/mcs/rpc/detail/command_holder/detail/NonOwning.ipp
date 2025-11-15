// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::rpc::detail::command_holder
{
  template<typename Command>
    constexpr NonOwning<Command>::NonOwning
      ( std::reference_wrapper<Command const> command
      ) noexcept
        : _command {command}
  {}

  template<typename Command>
    constexpr auto NonOwning<Command>::ref() const noexcept -> Command const&
  {
    return _command.get();
  }
}
