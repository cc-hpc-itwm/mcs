// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/TemporaryDirectory.hpp>

namespace mcs::util
{
  namespace
  {
    auto random_path() -> std::filesystem::path
    {
      return fmt::format ("{}", testing::random::value<std::uintmax_t>{}());
    }

    auto random_nonexisting_path
      ( std::filesystem::path const& root
      ) -> std::filesystem::path
    {
      if (auto path {root / random_path()}; !std::filesystem::exists (path))
      {
        return path;
      }

      return random_nonexisting_path (root);
    }

    auto random_nonexisting_path() -> std::filesystem::path
    {
      return random_nonexisting_path (std::filesystem::current_path());
    }

    auto create_random_content
      ( std::filesystem::path const& root
      , unsigned depth = 0u
      ) -> void
    {
      if (depth < 5u)
      {
        auto count {testing::random::value<unsigned> {0u, 8u}()};

        while (count --> 0u)
        {
          auto const path {random_nonexisting_path (root)};

          if (testing::random::value<unsigned>{0u, 99u}() < 33u)
          {
            std::filesystem::create_directory (path);

            create_random_content (path, depth + 1u);
          }
          else
          {
            std::ignore = std::ofstream {path};
          }
        }
      }
    }
  }

  TEST (TemporaryDirectory, is_move_constructible)
  {
    static_assert (std::is_move_constructible_v<TemporaryDirectory>);
  }
  TEST (TemporaryDirectory, is_move_assignable)
  {
    static_assert (std::is_move_assignable_v<TemporaryDirectory>);
  }
  TEST (TemporaryDirectory, is_not_copy_constructible)
  {
    static_assert (!std::is_copy_constructible_v<TemporaryDirectory>);
  }
  TEST (TemporaryDirectory, is_not_copy_assignable)
  {
    static_assert (!std::is_copy_assignable_v<TemporaryDirectory>);
  }

  TEST (TemporaryDirectory, non_existing_path_can_be_used_as_temporary_path)
  {
    auto const path {random_nonexisting_path()};

    ASSERT_FALSE (std::filesystem::exists (path));

    {
      auto const temp {TemporaryDirectory {path}};

      ASSERT_TRUE (std::filesystem::exists (path));

      ASSERT_EQ (temp.path(), path);
    }

    ASSERT_FALSE (std::filesystem::exists (path));
  }

  TEST (TemporaryDirectory, parameter_must_not_be_deeper_than_one)
  {
    auto const path {random_nonexisting_path()};

    ASSERT_FALSE (std::filesystem::exists (path));

    ASSERT_THROW
      ( std::ignore = TemporaryDirectory {path / random_path()}
      , std::system_error
      );

    ASSERT_FALSE (std::filesystem::exists (path));
  }

  TEST (TemporaryDirectory, all_content_is_removed_on_destruction)
  {
    auto const path {random_nonexisting_path()};

    ASSERT_FALSE (std::filesystem::exists (path));

    {
      auto const temp {TemporaryDirectory {path}};

      ASSERT_TRUE (std::filesystem::exists (path));

      create_random_content (temp.path());
    }

    ASSERT_FALSE (std::filesystem::exists (path));
  }

  TEST (TemporaryDirectory, existing_path_can_not_be_used_as_temporary_path)
  {
    auto const path {random_nonexisting_path()};

    ASSERT_FALSE (std::filesystem::exists (path));

    {
      auto const temp {TemporaryDirectory {path}};

      ASSERT_TRUE (std::filesystem::exists (path));

      testing::require_exception
        ( [&]
          {
            std::ignore = TemporaryDirectory {path};
          }
        , testing::Assert<NonExistingPath::Error::PathAlreadyExists>
            { [&] (auto const& caught)
              {
                ASSERT_EQ (caught.path(), path);
                ASSERT_STREQ
                  ( caught.what()
                  , fmt::format ( "NonExistingPath: '{}' already exists."
                                , path
                                ).c_str()
                  );
              }
            }
        );
    }

    ASSERT_FALSE (std::filesystem::exists (path));
  }

  TEST ( TemporaryDirectory
       , failed_constructor_call_does_not_delete_existing_directory
       )
  {
    auto const path {random_nonexisting_path()};

    ASSERT_FALSE (std::filesystem::exists (path));

    {
      auto const temp {TemporaryDirectory {path}};

      ASSERT_TRUE (std::filesystem::exists (path));

      {
        testing::require_exception
        ( [&]
          {
            std::ignore = TemporaryDirectory {path};
          }
        , testing::Assert<NonExistingPath::Error::PathAlreadyExists>
            { [&] (auto const& caught)
              {
                ASSERT_EQ (caught.path(), path);
                ASSERT_STREQ
                  ( caught.what()
                  , fmt::format ( "NonExistingPath: '{}' already exists."
                                , path
                                ).c_str()
                  );
              }
            }
        );
      }

      ASSERT_TRUE (std::filesystem::exists (path));
    }

    ASSERT_FALSE (std::filesystem::exists (path));
  }
}
