// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fcntl.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/open.hpp>
#include <system_error>
#include <tuple>

namespace
{
  TEST (MCSUtilSyscall, open_preserves_path_and_missing_file_error)
  {
    auto temporary {mcs::testing::UniqTemporaryDirectory {"MCSUtilSyscall"}};

    auto const path {temporary.path() / "missing"};

    mcs::testing::require_exception
      ( [&]
        {
          std::ignore = mcs::util::syscall::open (path.c_str(), O_RDONLY);
        }
      , mcs::testing::assert_type_and_what<mcs::util::syscall::Error>
          ( fmt::format
            ( "syscall::open (path = {}, flags = 0, mode = 0)"
            , mcs::util::FMT::STD::FILESYSTEM::Path {path}
            )
          )
      , mcs::testing::Assert<std::system_error>
          { [] (auto const& error)
            {
              EXPECT_EQ (error.code(), std::errc::no_such_file_or_directory);
            }
          }
      );
  }
}
