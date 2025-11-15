// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/Buffer.hpp>
#include <memory>

namespace mcs::util
{
  struct UtilBuffer : public testing::random::Test{};

  TEST_F
    ( UtilBuffer
    , size_is_stored_as_received_by_the_constructor_and_can_be_observed
    )
  {
    auto const size {testing::random::value<std::size_t>{}()};

    auto const buffer {Buffer<std::byte> {size}};

    ASSERT_EQ (buffer.size(), size);
    ASSERT_EQ (buffer.data<std::byte const>().size(), size);
  }

  TEST_F
    ( UtilBuffer
    , custom_deleter_gets_called_on_destruction
    )
  {
    auto call_count {0};
    auto value {testing::random::value<std::size_t>{}()};

    {
      auto const buffer
        { Buffer<void, std::function<void (void*)>>
            { testing::random::value<std::size_t>{}()
            , std::addressof (value)
            , [&] (auto* ptr)
              {
                ASSERT_EQ (ptr, std::addressof (value));

                ++call_count;
              }
            }
        };

      ASSERT_EQ (call_count, 0);
    }

    ASSERT_EQ (call_count, 1);
  }
}
