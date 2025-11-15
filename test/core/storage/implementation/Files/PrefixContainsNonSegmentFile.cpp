// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Parameter/Create.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Files/Prefix.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/CIdent.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/testing/until.hpp>
#include <mcs/util/FMT/write_file.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/type/List.hpp>
#include <utility>

namespace mcs::core
{
  namespace
  {
    struct MCSStorageFilesR : public testing::random::Test
    {
      testing::random::value<testing::random::CIdent> random_file_name{};
    };
  }

  TEST_F (MCSStorageFilesR, creation_throws_if_prefix_contains_non_segment_file)
  {
    auto const not_a_segment_name
      { testing::until
          ( [] (auto const& name)
            {
              try
              {
                std::ignore = util::read::read<storage::segment::ID> (name);
              }
              catch (...)
              {
                // can not be parsed as segment ID
                return true;
              }

              // can be parsed as segment ID, try another name
              return false;
            }
          , [&]
            {
              return random_file_name();
            }
          )
      };

    auto const temporary_directory
      { testing::UniqTemporaryDirectory
          {"MCS_TESTStorageFilesPrefixContainsNonSegmentFile"}
      };

    auto const non_segment_file
      {temporary_directory.path() / not_a_segment_name};

    util::FMT::write_file
      (non_segment_file, "{}", testing::random::value<std::string>{}());

    using Files = storage::implementation::Files;

    auto const prefix {Files::Prefix {temporary_directory.path()}};

    auto const parameter_create
      { Files::Parameter::Create
          { prefix
          , storage::MaxSize {storage::MaxSize::Unlimited{}}
          }
      };

    auto storages {Storages<util::type::List<Files>>{}};

    testing::require_exception
      ( [&]
        {
          std::ignore = storages.template create<Files>
                          ( storages.write_access()
                          , parameter_create
                          )
          ;
        }
      , testing::Assert<Files::Error::Create>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.parameter(), parameter_create);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "storage::implementation::Files::Files: {}"
                , parameter_create
                ).c_str()
              );
          }
        }
      , testing::Assert<Files::Error::Create::PrefixContainsNonSegmentFile>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.prefix(), prefix);
            ASSERT_EQ (caught.non_segment_file(), non_segment_file);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "storage::implementation::Files::PrefixContainsNonSegmentFile"
                ": prefix '{}', file '{}'"
                , prefix
                , non_segment_file
                ).c_str()
              );
          }
        }
      );
  }
}
