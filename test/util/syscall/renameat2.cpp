// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fcntl.h>
#include <gtest/gtest.h>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/renameat2.hpp>
#include <system_error>

namespace
{
  TEST ( MCSUtilSyscall
       , renameat2_errors_preserve_arguments_and_nested_error_codes
         )
  {
    mcs::testing::require_exception
      ( []
        {
          mcs::util::syscall::renameat2 (-1, "source", -1, "destination", 0);
        }
      , mcs::testing::assert_type_and_what<mcs::util::syscall::Error>
          ( "syscall::renameat2 (source_directory = -1, source = 'source',"
            " destination_directory = -1, destination = 'destination', flags = 0)"
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
