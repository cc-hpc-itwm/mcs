// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>

namespace mcs::rpc
{
  TEST (ScopedRunningIOContextNumberOfThreads, zero_throws)
  {
    using Expected =
      rpc::ScopedRunningIOContext::NumberOfThreads::Error::MustBePositive
      ;

    testing::require_exception
      ( []
        {
          std::ignore = ScopedRunningIOContext::NumberOfThreads {0u};
        }
      , testing::assert_type<Expected>()
      );
  }

  namespace
  {
    struct ReadOfFmtIsIdentityR : public testing::random::Test{};
  }

  TEST_F (ReadOfFmtIsIdentityR, rpc_ScopedRunningIOContextNumberOfThreads)
  {
    using RandomValue = testing::random::value<unsigned int>;
    auto const value {RandomValue {RandomValue::Min {1u}}()};

    testing::read_of_fmt_is_identity
      ( ScopedRunningIOContext::NumberOfThreads {value}
      , [&] (auto const& read)
        {
          return read.value == value;
        }
      );
  }
}
