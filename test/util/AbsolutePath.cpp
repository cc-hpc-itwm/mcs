// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <concepts>
#include <filesystem>
#include <gtest/gtest.h>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/AbsolutePath.hpp>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <tuple>

namespace
{
  static_assert
    ( std::convertible_to< std::filesystem::path
                         , mcs::util::AbsolutePath
                         >
    );

  TEST (UtilAbsolutePath, accepts_absolute_path)
  {
    auto const path {std::filesystem::path {"/absolute/path"}};
    auto const absolute_path {mcs::util::AbsolutePath {path}};

    EXPECT_EQ (absolute_path.path(), path);
  }

  TEST (UtilAbsolutePath, rejects_relative_path)
  {
    auto const path {std::filesystem::path {"relative/path"}};

    mcs::testing::require_exception
      ( [&]
        {
          std::ignore = mcs::util::AbsolutePath {path};
        }
      , mcs::testing::assert_type_and_what
          < mcs::util::AbsolutePath::Error::MustBeAbsolute
          > ( fmt::format ( "{} must be absolute"
                          , mcs::util::FMT::STD::FILESYSTEM::Path {path}
                          )
            )
      );
  }
}
