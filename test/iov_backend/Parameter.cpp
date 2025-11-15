// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <gtest/gtest.h>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/testing/iov_backend/operator==/Parameter.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>

namespace mcs::iov_backend
{
  TEST (ReadOfFmtIsIdentity, mcs_iov_backend_Parameter)
  {
    // \todo random<iov_backend::Parameter>
    testing::read_of_fmt_is_identity
      ( Parameter
          { util::ASIO::make_connectable (asio::ip::tcp::endpoint{})
          }
      );
  }
}
