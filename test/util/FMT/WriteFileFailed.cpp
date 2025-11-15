// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/write_file.hpp>

namespace mcs::util::FMT
{
  TEST (WriteFileFailed, thrown_as_outer_on_error)
  {
    auto const temporary_directory
      { testing::UniqTemporaryDirectory {"TEST_UTIL_FMT_WRITE_FILE_FAILED"}
      };
    auto const non_existing_directory
      {temporary_directory.path() / "directory"};

    ASSERT_FALSE (std::filesystem::exists (non_existing_directory));

    auto const non_existing_path {non_existing_directory / "path"};

    testing::require_exception
      ( [&]
        {
          write_file (non_existing_path, "");
        }
      , testing::Assert<WriteFileFailed>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.path(), non_existing_path);
          }
        }
      , testing::assert_type_or_derived_type<std::exception>()
      );
  }
}
