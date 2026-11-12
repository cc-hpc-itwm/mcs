// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/CIdent.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <memory>

namespace mcs::testing
{
  struct UniqTemporaryDirectoryF : public testing::random::Test{};

  TEST_F (UniqTemporaryDirectoryF, writes_diagnostic_files)
  {
    auto prefix {random::value<random::CIdent>{}()};
    auto temp {std::make_unique<UniqTemporaryDirectory> (prefix)};

    auto const path {temp->path()};
    auto const parent {path.parent_path()};
    auto const test_file {parent / "TEST"};
    auto const process_id_file {parent / "PID"};
    auto const prefix_file {parent / "PREFIX"};
    auto const directory_name {parent.filename().string()};

    ASSERT_TRUE (std::filesystem::exists (path));
    ASSERT_TRUE (std::filesystem::is_directory (path));
    ASSERT_TRUE (directory_name.starts_with ("MCS-TEST-"));
    ASSERT_EQ (directory_name.size(), 41UL);
    ASSERT_TRUE (std::filesystem::exists (parent));
    ASSERT_TRUE (std::filesystem::is_directory (parent));
    ASSERT_TRUE (std::filesystem::exists (test_file));
    ASSERT_TRUE (std::filesystem::is_regular_file (test_file));
    ASSERT_TRUE (std::filesystem::exists (process_id_file));
    ASSERT_TRUE (std::filesystem::is_regular_file (process_id_file));
    ASSERT_TRUE (std::filesystem::exists (prefix_file));
    ASSERT_TRUE (std::filesystem::is_regular_file (prefix_file));
    ASSERT_FALSE (std::filesystem::exists (parent / "INFO"));

    auto read_file
      { [] (std::filesystem::path const& file)
        {
          auto input {std::ifstream {file}};
          input >> std::noskipws;
          return std::string
            { std::istream_iterator<char> (input)
            , std::istream_iterator<char>()
            };
        }
      };
    ASSERT_EQ
      ( read_file (test_file)
      , "UniqTemporaryDirectoryF.writes_diagnostic_files\n"
      );
    ASSERT_EQ
      ( read_file (process_id_file)
      , fmt::format ("{}\n", util::syscall::getpid())
      );
    ASSERT_EQ (read_file (prefix_file), fmt::format ("{}\n", prefix));

    temp.reset();

    ASSERT_FALSE (std::filesystem::exists (path));
    ASSERT_FALSE (std::filesystem::exists (parent));
    ASSERT_FALSE (std::filesystem::exists (test_file));
    ASSERT_FALSE (std::filesystem::exists (process_id_file));
    ASSERT_FALSE (std::filesystem::exists (prefix_file));

    auto recreated {std::make_unique<UniqTemporaryDirectory> (prefix)};
    ASSERT_EQ (recreated->path().parent_path(), parent);
  }
}
