// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <span>
#include <vector>

namespace
{
  struct Append
  {
    std::vector<int> xs;
  };

  struct Stream
  {
    std::vector<int> xs;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<Append>
  {
    using Type = Append;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
  template<>
    struct Implementation<Stream>
  {
    using Type = Stream;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::serialization
{
  auto Implementation<Append>::output (OArchive& oa, Append const& x) -> OArchive&
  {
    save (oa, x.xs.size());
    oa.append (std::span {x.xs});

    return oa;
  }
  auto Implementation<Stream>::output (OArchive& oa, Stream const& x) -> OArchive&
  {
    save (oa, x.xs.size());
    oa.stream (std::span {x.xs});

    return oa;
  }
}

namespace mcs::serialization
{
  struct Serialization : public testing::random::Test{};

  TEST_F (Serialization, stream_does_not_increase_size)
  {
    auto xs {std::vector<int>{}};
    xs.resize (testing::random::value<std::size_t> {0, 4 << 10}());
    std::ranges::generate
      ( xs
      , [random_int = testing::random::value<int>{}]() mutable
        {
          return random_int();
        }
      );

    auto const append {Append {xs}};
    auto const stream {Stream {xs}};

    auto const oa_append {OArchive {append}};
    auto const oa_stream {OArchive {stream}};

    auto const& append_buffers {oa_append.buffers()};
    auto const& stream_buffers {oa_stream.buffers()};

    // the size of the vector and the vector elements are stored in
    // the same way for both objects

    ASSERT_EQ (append_buffers.size(), 3);
    ASSERT_EQ (stream_buffers.size(), 3);

    auto const append_span
      { [&] (std::size_t i)
        {
          return std::as_bytes (append_buffers.at (i));
        }
      };
    auto const stream_span
      { [&] (std::size_t i)
        {
          return std::as_bytes (stream_buffers.at (i));
        }
      };

    ASSERT_THAT (append_span (1), ::testing::ElementsAreArray (stream_span (1)));
    ASSERT_THAT (append_span (2), ::testing::ElementsAreArray (stream_span (2)));

    auto const raw_span
      { [&] (auto& size)
        {
          return std::as_bytes (std::span {std::addressof (size), 1});
        }
      };

    // the size of the archive is different though: for append it is
    // the sum of all sizes, for stream the streaming data does not
    // account for the archive size
    {
      auto size {sizeof (xs.size()) + xs.size() * sizeof (int)};
      ASSERT_THAT
        ( append_span (0)
        , ::testing::ElementsAreArray (raw_span (size))
        );
    }
    {
      auto size {sizeof (xs.size())};
      ASSERT_THAT
        ( stream_span (0)
        , ::testing::ElementsAreArray (raw_span (size))
        );
    }
  }
}
