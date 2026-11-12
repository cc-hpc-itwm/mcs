// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <cstdlib>
#include <gtest/gtest.h>
#include <mcs/fuse/content/implementation/Vector.hpp>
#include <mcs/fuse/main.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/arguments/View.hpp>
#include <stdexcept>
#include <string>

namespace
{
  TEST (FuseMain, help_succeeds_without_signaling_ready)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers {std::array<char const*, 2> {"filesystem", "--help"}};
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };

    auto usage_count {0};
    auto ready {false};

    auto const result
      { mcs::fuse::main<Content>
        ( [&usage_count]() noexcept
          {
            ++usage_count;
          }
        , arguments
        , Content::State{}
        , [&ready]() noexcept
          {
            ready = true;
          }
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_EQ (usage_count, 1);
    EXPECT_FALSE (ready);

    auto const expected {std::array<std::string, 2> {"filesystem", "--help"}};
    EXPECT_TRUE (std::ranges::equal (arguments, expected));
  }

  TEST (FuseMain, convenience_overload_accepts_arguments)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers {std::array<char const*, 2> {"filesystem", "-h"}};
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };

    auto usage_count {0};

    auto const result
      { mcs::fuse::main<Content>
        ( [&usage_count]() noexcept
          {
            ++usage_count;
          }
        , arguments
        , Content::State{}
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_EQ (usage_count, 1);
  }

  TEST (FuseMain, version_succeeds_without_usage_or_readiness)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers
      { std::array<char const*, 2> {"filesystem", "--version"}
      };
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };

    auto usage_count {0};
    auto ready {false};

    testing::internal::CaptureStdout();

    auto const result
      { mcs::fuse::main<Content>
        ( [&usage_count]() noexcept
          {
            ++usage_count;
          }
        , arguments
        , Content::State{}
        , [&ready]() noexcept
          {
            ready = true;
          }
        )
      };

    auto const output {testing::internal::GetCapturedStdout()};

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_EQ (usage_count, 0);
    EXPECT_FALSE (ready);

    EXPECT_NE (output.find ("FUSE library version "), std::string::npos);
  }

  TEST (FuseMain, missing_mountpoint_fails_without_signaling_ready)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers {std::array<char const*, 1> {"filesystem"}};
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };

    auto usage_count {0};
    auto ready {false};

    auto const result
      { mcs::fuse::main<Content>
        ( [&usage_count]() noexcept
          {
            ++usage_count;
          }
        , arguments
        , Content::State{}
        , [&ready]() noexcept
          {
            ready = true;
          }
        )
      };

    EXPECT_EQ (result, EXIT_FAILURE);
    EXPECT_EQ (usage_count, 1);
    EXPECT_FALSE (ready);
  }

  TEST (FuseMain, invalid_options_fail_without_signaling_ready)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers {std::array<char const*, 2> {"filesystem", "-o"}};
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };
    auto usage_count {0};
    auto ready {false};

    testing::internal::CaptureStderr();
    auto const result
      { mcs::fuse::main<Content>
        ( [&usage_count]() noexcept
          {
            ++usage_count;
          }
        , arguments
        , Content::State{}
        , [&ready]() noexcept
          {
            ready = true;
          }
        )
      };
    auto const error {testing::internal::GetCapturedStderr()};

    EXPECT_EQ (result, EXIT_FAILURE);
    EXPECT_EQ (usage_count, 1);
    EXPECT_FALSE (ready);
    EXPECT_FALSE (error.empty());

    auto const expected {std::array<std::string, 2> {"filesystem", "-o"}};
    EXPECT_TRUE (std::ranges::equal (arguments, expected));
  }

  TEST (FuseMain, usage_exceptions_propagate)
  {
    using Content = mcs::fuse::content::implementation::Vector;

    auto const pointers {std::array<char const*, 2> {"filesystem", "--help"}};
    auto const arguments
      { mcs::util::arguments::View {pointers.data(), pointers.size()}
      };

    mcs::testing::require_exception
      ( [&]
        {
          auto const result
            { mcs::fuse::main<Content>
              ( []() -> void
                {
                  throw std::runtime_error {"printing usage failed"};
                }
              , arguments
              , Content::State{}
              )
            };

          EXPECT_EQ (result, EXIT_SUCCESS);
        }
      , mcs::testing::assert_type_and_what<std::runtime_error>
          ("printing usage failed")
      );
  }
}
