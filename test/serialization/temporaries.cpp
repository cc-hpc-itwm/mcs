// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::serialization
{
  namespace
  {
    struct S
    {
      constexpr explicit S (int i)
        : _i {i}
      {}

      [[nodiscard]] constexpr auto value() const noexcept -> int
      {
        return _i;
      }

      int _i;
    };

    struct Serialization : public testing::random::Test{};
  }

  TEST_F ( Serialization
         , temporaries_can_be_serialized
         )
  {
    auto const value {testing::random::value<int>{}()};

    auto const bytes
      { [&]
        {
          auto const s {S {value}};
          auto oa {OArchive {s.value()}};
          return oa.bytes();
        }()
      };

    auto const deserialized_value {load_from<int> (bytes.data(), bytes.size())};

    ASSERT_EQ (deserialized_value, value);
  }
}
