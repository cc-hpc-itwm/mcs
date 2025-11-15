// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/ASIO/SetSocketOptions.hpp>

namespace mcs::util::ASIO
{
  auto SetSocketOptions<asio::ip::tcp>::operator()
    ( asio::ip::tcp::socket& socket
    ) const -> void
  {
    socket.set_option (asio::ip::tcp::no_delay (true));
  }
}
