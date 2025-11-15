// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/CIdent.hpp>
#include <memory>

namespace mcs::testing
{
  struct UniqTemporaryDirectoryF : public testing::random::Test{};

  TEST_F (UniqTemporaryDirectoryF, creates_INFO_file)
  {
    auto prefix {random::value<random::CIdent>{}()};
    auto temp {std::make_unique<UniqTemporaryDirectory> (prefix)};

    auto const path {temp->path()};
    auto const parent {path.parent_path()};
    auto const info {parent / "INFO"};

    ASSERT_TRUE (std::filesystem::exists (path));
    ASSERT_TRUE (std::filesystem::is_directory (path));
    ASSERT_TRUE (std::filesystem::exists (parent));
    ASSERT_TRUE (std::filesystem::is_directory (parent));
    ASSERT_TRUE (std::filesystem::exists (info));
    ASSERT_TRUE (std::filesystem::is_regular_file (info));

    {
      std::ifstream ifs {info};
      ifs >> std::noskipws;
      auto const content
        { std::string { std::istream_iterator<char> (ifs)
                      , std::istream_iterator<char>()
                      }
        };

      ASSERT_EQ (content, "UniqTemporaryDirectoryF.creates_INFO_file\n");
    }

    temp.reset();

    ASSERT_FALSE (std::filesystem::exists (path));
    ASSERT_FALSE (std::filesystem::exists (parent));
    ASSERT_FALSE (std::filesystem::exists (info));
  }
}
