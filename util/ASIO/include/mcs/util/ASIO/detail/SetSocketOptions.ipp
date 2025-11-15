// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::ASIO
{
  constexpr auto SetSocketOptions<asio::local::stream_protocol>::operator()
    ( asio::local::stream_protocol::socket&
    ) const noexcept -> void
  {
    // do nothing
  }
}
