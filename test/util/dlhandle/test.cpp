// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "library.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/DLHandle.hpp>
#include <mcs/util/syscall/Error.hpp>

namespace mcs::util
{
  TEST (UtilDLHandle, missing_file_throws)
  {
    auto const nonexisting_path {"nonexisting path"};

    testing::require_exception
      ( [&]
        {
          std::ignore = DLHandle {nonexisting_path};
        }
      , testing::assert_type_and_what<syscall::Error>
        ( fmt::format ( "syscall::dlopen (filename = '{}', flag = {})"
                      , nonexisting_path
                      , DLHandle::flags_default
                      )
        )
      , testing::Assert<syscall::error::DLError>
          { [&] (auto const& caught)
            {
              ASSERT_EQ (caught.call(), "dlopen");
              ASSERT_TRUE
                (caught.reason().ends_with ("No such file or directory"));
            }
          }
      );
  }

  TEST (UtilDLHandle, missing_symbol_throws)
  {
    auto const dlhandle {DLHandle {MCS_TEST_UTIL_DLHANDLE_LIBRARY}};
    auto const nonexisting_symbol {"nonexisting symbol"};

    testing::require_exception
      ( [&]
        {
          dlhandle.symbol<void> (nonexisting_symbol);
        }
      , testing::assert_type_and_what<DLHandle::Error::Symbol>
          ( fmt::format ("DLHandle::symbol ({})", nonexisting_symbol)
          )
      , testing::Assert<syscall::Error>
        { [&] (auto const& caught)
          {
            ASSERT_TRUE
              ( std::string {caught.what()}.starts_with
                ( "syscall::dlsym (handle = "
                )
              );
            ASSERT_TRUE
              ( std::string {caught.what()}.ends_with
                ( fmt::format ( ", symbol = '{}')", nonexisting_symbol)
                )
              );
          }
        }
      , testing::Assert<syscall::error::DLError>
          { [&] (auto const& caught)
            {
              ASSERT_EQ (caught.call(), "dlsym");
              ASSERT_TRUE (caught.reason().ends_with (nonexisting_symbol));
            }
          }
      );
  }
}

namespace mcs::util
{
  namespace
  {
    struct UtilDLHandleR : public testing::random::Test{};
  }

  TEST_F (UtilDLHandleR, symbols_are_possible_to_be_retrieved_and_called)
  {
    auto const dlhandle {DLHandle {MCS_TEST_UTIL_DLHANDLE_LIBRARY}};

    auto value (MCS_UTIL_DLHANDLE_SYMBOL (dlhandle, dltest_value));
    auto const set (MCS_UTIL_DLHANDLE_SYMBOL (dlhandle, dltest_set));
    auto const get (MCS_UTIL_DLHANDLE_SYMBOL (dlhandle, dltest_get));

    ASSERT_EQ (*value, -1);

    auto const x {testing::random::value<int>{}()};
    *value = x;
    ASSERT_EQ (get(), x);

    auto const y {testing::random::value<int>{}()};
    ASSERT_EQ (set (y), y);
    ASSERT_EQ (get(), y);
    ASSERT_EQ (*value, y);
  }

  TEST_F (UtilDLHandleR, multiple_handles_can_exist_at_the_same_time)
  {
    auto const dlhandle0 {DLHandle {MCS_TEST_UTIL_DLHANDLE_LIBRARY}};

    auto value (MCS_UTIL_DLHANDLE_SYMBOL (dlhandle0, dltest_value));
    auto const x {testing::random::value<int>{}()};

    {
      auto const dlhandle1 {DLHandle {MCS_TEST_UTIL_DLHANDLE_LIBRARY}};

      std::invoke (MCS_UTIL_DLHANDLE_SYMBOL (dlhandle1, dltest_set), x);
      ASSERT_EQ (*value, x);
    }

    ASSERT_EQ (*value, x);
  }
}
