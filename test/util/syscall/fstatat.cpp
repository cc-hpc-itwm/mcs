// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fcntl.h>
#include <gtest/gtest.h>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/fstatat.hpp>
#include <system_error>
#include <tuple>

namespace
{
  TEST ( MCSUtilSyscall
       , fstatat_errors_preserve_arguments_and_nested_error_codes
       )
  {
    mcs::testing::require_exception
      ( []
        {
          std::ignore = mcs::util::syscall::fstatat (-1, "file", 0);
        }
      , mcs::testing::assert_type_and_what<mcs::util::syscall::Error>
          ( "syscall::fstatat (directory = -1, path = 'file', flags = 0)"
          )
      , mcs::testing::Assert<std::system_error>
          { [] (auto const& error)
            {
              EXPECT_EQ (error.code(), std::errc::bad_file_descriptor);
            }
          }
      );
  }
}
