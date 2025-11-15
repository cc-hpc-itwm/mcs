// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Virtual/Parameter/Create.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/random/storage/implementation/Virtual/Parameter/Create.hpp>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/testing/until.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/dlerror.hpp>
#include <mcs/util/type/List.hpp>
#include <utility>
#include <vector>

namespace mcs::core::storage::implementation
{
  TEST (MCSStorageVirtual, is_a_storage_implementation)
  {
    static_assert (is_implementation<Virtual>);
  }

  struct MCSStorageVirtualF : public ::testing::Test{};

  TEST_F (MCSStorageVirtualF, parameter_create_is_serializable)
  {
    static_assert
      (serialization::is_serializable<typename Virtual::Parameter::Create>);

    testing::deserialized_from_serialized_is_identity
      ( std::invoke (testing::random::value<Virtual::Parameter::Create>{})
      );
  }
  TEST_F (MCSStorageVirtualF, parameter_create_is_fmt_and_read_able)
  {
    static_assert
      (fmt::is_formattable<typename Virtual::Parameter::Create>::value);
    static_assert (util::read::is_readable<typename Virtual::Parameter::Create>);

    testing::read_of_fmt_is_identity
      ( std::invoke (testing::random::value<Virtual::Parameter::Create>{})
      );
  }

  TEST_F (MCSStorageVirtualF, construction_from_a_nonexisting_file_throws)
  {
    auto const temporary_path {testing::UniqTemporaryDirectory {""}};
    auto const non_existing_file
      { testing::until
        ( [] (auto const& file)
          {
            return !std::filesystem::exists (file);
          }
        , [&]
          {
            return temporary_path.path()
              / std::to_string (std::invoke (testing::random::value<int>{}))
              ;
          }
        )
      };
    auto const parameter_create
      { Virtual::Parameter::Create
        { non_existing_file
        , std::invoke (testing::random::value<Virtual::StorageParameter>{})
        }
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = Virtual {parameter_create};
        }
      , testing::Assert<Virtual::Error::Create>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.parameter(), parameter_create);
          }
        }
      , testing::assert_type_or_derived_type<mcs::util::syscall::Error>()
      , testing::assert_type<util::syscall::error::DLError>()
      );
  }

  TEST_F (MCSStorageVirtualF, storage_chunky_create_failure_throws)
  {
    // \note The implementation chunky expects the max_size
    // encoded in the parameter and throws the content of the
    // parameter if it has the wrong size.
    auto const parameter_create
      { Virtual::Parameter::Create
        { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
        , testing::until
          ( [] (auto const& parameter)
            {
              return parameter.size() != sizeof (::MCS_CORE_STORAGE_SIZE);
            }
          , testing::random::value<Virtual::StorageParameter>{}
          )
        }
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = Virtual {parameter_create};
        }
      , testing::Assert<Virtual::Error::Create>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.parameter(), parameter_create);
          }
        }
      , testing::Assert<Import_C_API::Error::Create>
        { [&] (auto const& caught)
          {
            ASSERT_EQ ( caught.parameter()._parameter_create
                      , parameter_create._parameter_create
                      );
          }
        }
      , testing::Assert<Import_C_API::Error::Implementation>
        { [&] (auto const& caught)
          {
            ASSERT_STREQ
              ( util::cast<char const*> (caught.error().data())
              , "chunky::construct: parameter has the wrong size"
              );
          }
        }
      );
  }

  namespace
  {
    auto make_implementation_parameter_create (::MCS_CORE_STORAGE_SIZE size)
    {
      auto parameter {Virtual::StorageParameter{}};
      parameter.reserve (sizeof (size));

      auto size_data
        { util::cast<::MCS_CORE_STORAGE_BYTE const*> (std::addressof (size))
        };

      parameter.insert
        ( std::end (parameter)
        , size_data
        , size_data + sizeof (size)
        );

      return parameter;
    }

    constexpr auto make_max_size
      ( ::MCS_CORE_STORAGE_SIZE size
      ) -> MaxSize
    {
      if (size == ::MCS_CORE_STORAGE_SIZE {0})
      {
        return storage::MaxSize::Unlimited{};
      }

      return storage::MaxSize::Limit {memory::make_size (size)};
    }
  }

  TEST_F (MCSStorageVirtualF, storage_chunky_remembers_size_max)
  {
    auto const max_size
      { std::invoke (testing::random::value<::MCS_CORE_STORAGE_SIZE>{})
      };
    auto const vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (max_size)
          }
        }
      };
    ASSERT_EQ
      ( vstorage.size_max
        ( Virtual::Parameter::Size::Max
          { std::invoke (testing::random::value<Virtual::StorageParameter>{})
          }
        )
      , make_max_size (max_size)
      );
  }

  TEST_F (MCSStorageVirtualF, storage_chunky_initially_uses_zero_bytes)
  {
    auto const max_size
      {std::invoke (testing::random::value<::MCS_CORE_STORAGE_SIZE>{})};
    auto const vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (max_size)
          }
        }
      };
    ASSERT_EQ
      ( vstorage.size_used
        ( Virtual::Parameter::Size::Used
          { std::invoke (testing::random::value<Virtual::StorageParameter>{})
          }
        )
      , memory::make_size (0)
      );
  }

  TEST_F ( MCSStorageVirtualF
         , to_create_a_segment_in_storage_chunky_throws_bad_alloc_if_the_parameter_is_not_empty
         )
  {
    auto const max_size
      { std::invoke (testing::random::value<::MCS_CORE_STORAGE_SIZE>{})
      };
    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (max_size)
          }
        }
      };

    auto const nonempty_parameter_segment_create
      { std::invoke
        ( []
          {
            auto _parameter_segment_create
              { std::invoke
                ( testing::random::value<Virtual::StorageParameter>{}
                )
              };

            // make sure there is at least one byte in the parameter
            _parameter_segment_create.emplace_back
              ( std::invoke
                ( testing::random::value<::MCS_CORE_STORAGE_BYTE>{}
                )
              );

            return _parameter_segment_create;
          }
        )
      };
    auto const size {std::invoke (testing::random::value<memory::Size>{})};

    testing::require_exception
      ( [&]
        {
          std::ignore = vstorage.segment_create
            ( Virtual::Parameter::Segment::Create
              { nonempty_parameter_segment_create
              }
            , size
            );
        }
      , testing::Assert<Virtual::Error::BadAlloc>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.requested(), size);
            ASSERT_EQ (caught.used(), memory::make_size (0));
            ASSERT_EQ (caught.max(), make_max_size (max_size));
          }
        }
      );
  }

  TEST_F ( MCSStorageVirtualF
         , to_create_a_segment_in_storage_chunky_does_not_increase_size_used
         )
  {
    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create
            ( std::invoke (testing::random::value<::MCS_CORE_STORAGE_SIZE>{})
            )
          }
        }
      };

    std::ignore = vstorage.segment_create
      ( Virtual::Parameter::Segment::Create
        { Virtual::StorageParameter{}
        }
      , std::invoke (testing::random::value<memory::Size>{})
      );

    auto const size_used
      { vstorage.size_used
        ( Virtual::Parameter::Size::Used
          { std::invoke (testing::random::value<Virtual::StorageParameter>{})
          }
        )
      };

    ASSERT_EQ (size_used, memory::make_size (0));
  }

  TEST_F ( MCSStorageVirtualF
         , to_remove_a_segment_from_storage_chunky_is_not_implemented
         )
  {
    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (::MCS_CORE_STORAGE_SIZE {0})
          }
        }
      };

    auto const segment_id
      { vstorage.segment_create
        ( Virtual::Parameter::Segment::Create
          { Virtual::StorageParameter{}
          }
        , std::invoke (testing::random::value<memory::Size>{})
        )
      };

    auto const implementation_parameter
      { std::invoke (testing::random::value<Virtual::StorageParameter>{})
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = vstorage.segment_remove
            ( Virtual::Parameter::Segment::Remove
              { implementation_parameter
              }
            , segment_id
            );
        }
      , testing::Assert<Virtual::Error::Segment::Remove>
        { [&] (auto const& caught)
          {
            ASSERT_EQ ( caught.parameter()._parameter_segment_remove
                      , implementation_parameter
                      );
            ASSERT_EQ (caught.segment_id(), segment_id);
          }
        }
      , testing::assert_type<Virtual::Error::MethodNotProvided>()
      );
  }

  TEST_F ( MCSStorageVirtualF
         , storage_chunky_allows_to_create_a_chunk_and_that_chunk_is_filled_with_the_value_provided_in_the_parameter_chunk_description
         )
  {
    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (::MCS_CORE_STORAGE_SIZE {0})
          }
        }
      };

    auto const segment_id
      { vstorage.segment_create
        ( Virtual::Parameter::Segment::Create
          { Virtual::StorageParameter{}
          }
        , std::invoke (testing::random::value<memory::Size>{})
        )
      };

    using RandomSize = testing::random::value<::MCS_CORE_STORAGE_SIZE>;
    auto const size
      { std::invoke (RandomSize { RandomSize::Min {1 << 10}
                                , RandomSize::Max {256 << 20}
                                }
                    )
      };
    auto const value
      { std::invoke
        ( testing::random::value<::MCS_CORE_STORAGE_BYTE>{}
        )
      };

    auto const parameter_chunk_description {Virtual::StorageParameter {value}};

    auto const chunk_description
      { vstorage.template chunk_description<chunk::access::Const>
        ( Virtual::Parameter::Chunk::Description {parameter_chunk_description}
        , segment_id
        , memory::make_range
          ( memory::make_offset (0)
          , memory::make_size (size)
          )
        )
      };

    using SupportedStorageImplementations = util::type::List<Virtual>;

    SupportedStorageImplementations::template wrap
      < core::Chunk
      , chunk::access::Const
      > const chunk {chunk_description}
      ;

    auto const bytes {as<::MCS_CORE_STORAGE_BYTE const> (chunk)};

    ASSERT_EQ (bytes.size(), size);

    for (auto byte : bytes)
    {
      ASSERT_EQ (byte, value);
    }
  }

  TEST_F ( MCSStorageVirtualF
         , storage_chunky_maintains_size_used_as_sum_of_the_sizes_of_chunks
         )
  {
    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (::MCS_CORE_STORAGE_SIZE {0})
          }
        }
      };

    auto const segment_id
      { vstorage.segment_create
        ( Virtual::Parameter::Segment::Create
          { Virtual::StorageParameter{}
          }
        , std::invoke (testing::random::value<memory::Size>{})
        )
      };

    using RandomSize = testing::random::value<::MCS_CORE_STORAGE_SIZE>;
    auto random_size { RandomSize { RandomSize::Min {1 << 10}
                                  , RandomSize::Max {256 << 20}
                                  }
                     };
    auto const size1 {std::invoke (random_size)};
    auto const size2 {std::invoke (random_size)};

    auto random_value
      { testing::random::value<::MCS_CORE_STORAGE_BYTE>{}
      };

    auto const make_chunk_description
      { [&] (auto size)
        {
          return vstorage.template chunk_description<chunk::access::Const>
            ( Virtual::Parameter::Chunk::Description
              { Virtual::StorageParameter {std::invoke (random_value)}
              }
            , segment_id
            , memory::make_range
              ( memory::make_offset (0)
              , memory::make_size (size)
              )
            );
        }
      };

    auto const size_used
      { [&]
        {
          return vstorage.size_used
            ( Virtual::Parameter::Size::Used
              { Virtual::StorageParameter{}
              }
            );
        }
      };

    using SupportedStorageImplementations = util::type::List<Virtual>;

    ASSERT_EQ (std::invoke (size_used), memory::make_size (0));

    {
      SupportedStorageImplementations::template wrap
        < core::Chunk
        , chunk::access::Const
        > const chunk1 {std::invoke (make_chunk_description, size1)}
        ;

      ASSERT_EQ (std::invoke (size_used), memory::make_size (size1));

      {
        SupportedStorageImplementations::template wrap
          < core::Chunk
          , chunk::access::Const
          > const chunk2 {std::invoke (make_chunk_description, size2)}
          ;

        ASSERT_EQ (std::invoke (size_used), memory::make_size (size1 + size2));
      }

      ASSERT_EQ (std::invoke (size_used), memory::make_size (size1));
    }

    ASSERT_EQ (std::invoke (size_used), memory::make_size (0));
  }

  TEST_F ( MCSStorageVirtualF
         , storage_chunky_fails_to_create_more_chunks_if_that_would_exceed_the_size_max
         )
  {
    using RandomSize = testing::random::value<::MCS_CORE_STORAGE_SIZE>;
    auto random_size { RandomSize { RandomSize::Min {1 << 10}
                                  , RandomSize::Max {256 << 20}
                                  }
                     };
    auto const max {std::invoke (random_size)};

    auto vstorage
      { Virtual
        { Virtual::Parameter::Create
          { MCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY
          , make_implementation_parameter_create (max)
          }
        }
      };

    auto const segment_id
      { vstorage.segment_create
        ( Virtual::Parameter::Segment::Create
          { Virtual::StorageParameter{}
          }
        , std::invoke (testing::random::value<memory::Size>{})
        )
      };

    auto random_value
      { testing::random::value<::MCS_CORE_STORAGE_BYTE>{}
      };

    auto const make_chunk_description
      { [&] (auto size)
        {
          return vstorage.template chunk_description<chunk::access::Const>
            ( Virtual::Parameter::Chunk::Description
              { Virtual::StorageParameter {std::invoke (random_value)}
              }
            , segment_id
            , memory::make_range
              ( memory::make_offset (0)
              , memory::make_size (size)
              )
            );
        }
      };

    using SupportedStorageImplementations = util::type::List<Virtual>;
    using Chunk = SupportedStorageImplementations::template wrap
        < core::Chunk
        , chunk::access::Const
        >;

    auto chunks {std::list<Chunk>{}};

    auto size_used {::MCS_CORE_STORAGE_SIZE {0}};

    auto const make_chunk
      { [&] (auto size)
        {
          chunks.emplace_back (std::invoke (make_chunk_description, size));
        }
      };

    while (size_used < max)
    {
      auto const size {random_size()};

      if (size_used + size > max)
      {
        testing::require_exception
          ( [&]
            {
              make_chunk (size);
            }
          , testing::assert_type_and_what<Virtual::Error::Implementation>
            ( "storage::Import_C_API::Implementation:"
              " chunky::chunk_const_state:"
              " not enough memory available in storage"
            )
          );
      }
      else
      {
        make_chunk (size);
      }

      size_used += size;
    }
  }
}
