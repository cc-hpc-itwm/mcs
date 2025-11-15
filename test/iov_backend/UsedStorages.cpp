// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <deque>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/provider/distribution/AsEqualAsPossible.hpp>
#include <mcs/testing/core/random/memory/Range.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/one_of.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <queue>
#include <ranges>
#include <vector>

namespace mcs::iov_backend
{
  namespace
  {
    auto equally_distributed
      ( std::size_t number_of_storages
      , core::memory::Size size
      )
    {
      using provider::distribution::AsEqualAsPossible;

      return AsEqualAsPossible{}
        ( size
        , std::invoke
          ( [number_of_storages]
            {
              auto storage_ids_and_capacities
                { std::priority_queue<AsEqualAsPossible::StorageIDAndCapacity>{}
                };

              std::ranges::for_each
                ( std::ranges::iota_view {std::size_t {0}, number_of_storages}
                , [&, storage_id {storage::ID{}}] (auto) mutable
                  {
                    storage_ids_and_capacities.emplace
                      ( storage_id
                      , core::storage::MaxSize
                        { core::storage::MaxSize::Unlimited{}
                        }
                      );

                    ++storage_id;
                  }
                );

              return storage_ids_and_capacities;
            }
          )
        );
    }

    auto full_range (auto distribution)
    {
      auto const [smallest, largest]
        { std::ranges::minmax_element (std::views::values (distribution))
        };

      return core::memory::make_range
        ( core::memory::begin (*smallest)
        , core::memory::end (*largest)
        );
    }

    auto shift (auto distribution, core::memory::Offset offset)
    {
      auto shifted_distribution {distribution};

      std::ranges::for_each
        ( shifted_distribution
        , [offset] (auto& storage_id_and_range)
          {
            auto& range {storage_id_and_range.second};

            range = core::memory::shift (range, offset);
          }
        );

      return shifted_distribution;
    }

    auto make_used_storage
      ( auto storage_id_and_range
      , core::storage::segment::ID segment_id = {}
      )
    {
      auto const [storage_id, range] {storage_id_and_range};

      return UsedStorage {range, segment_id, storage_id};
    }

    auto make_constructor (auto distribution)
    {
      auto constructor {UsedStorages::Constructor{}};

      std::ranges::for_each
        ( distribution
        , [&] (auto storage_id_and_range)
          {
            constructor.emplace (make_used_storage (storage_id_and_range));
          }
        );

      return constructor;
    }

    auto random_inner_range (core::memory::Range range)
    {
      using RandomOffset = testing::random::value<core::memory::Offset>;
      auto random_offset
        { RandomOffset
          { RandomOffset::Min
            { offset_cast<core::memory::Offset::underlying_type>
              ( core::memory::begin (range)
              )
            }
          , RandomOffset::Max
            { offset_cast<core::memory::Offset::underlying_type>
              ( core::memory::end (range)
              )
            }
          }
        };
      auto const x {std::invoke (random_offset)};
      auto const y {std::invoke (random_offset)};

      return core::memory::make_range (std::min (x, y), std::max (x, y));
    }
  }

  namespace
  {
    struct MCSIOVBackendUsedStorageR : public testing::random::Test
    {
      using RandomNumberOfStorages = testing::random::value<std::size_t>;
      RandomNumberOfStorages random_number_of_storages
        { RandomNumberOfStorages
          { RandomNumberOfStorages::Min {1}
          , RandomNumberOfStorages::Max {100}
          }
        };
    };
  }

  TEST (MCSIOVBackendUsedStorage, constructed_with_empty_constructor_is_empty)
  {
    ASSERT_TRUE (UsedStorages {UsedStorages::Constructor{}}.empty());
  }

  TEST (MCSIOVBackendUsedStorage, if_empty_then_begin_equals_end)
  {
    auto const used_storages {UsedStorages {UsedStorages::Constructor{}}};

    ASSERT_EQ (used_storages.begin(), used_storages.end());
  }

  TEST_F (MCSIOVBackendUsedStorageR, if_empty_then_lower_bound_equals_end)
  {
    auto const used_storages {UsedStorages {UsedStorages::Constructor{}}};
    auto const random_range
      { std::invoke (testing::random::value<core::memory::Range>{})
      };

    ASSERT_EQ (used_storages.lower_bound (random_range), used_storages.end());
  }

  TEST (MCSIOVBackendUsedStorage, if_empty_then_range_throws)
  {
    testing::require_exception
      ( []
        {
          std::ignore = UsedStorages {UsedStorages::Constructor{}}.range();
        }
      , testing::assert_type_and_what<UsedStorages::Error::Range::Empty>
        ( "UsedStorages::range(): Empty."
        )
      );
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , constructed_with_non_empty_constructor_is_not_empty
         )
  {
    auto const distribution
      { equally_distributed
        ( std::invoke (random_number_of_storages)
        , std::invoke (testing::random::value<core::memory::Size>{})
        )
      };

    ASSERT_FALSE (UsedStorages {make_constructor (distribution)}.empty());
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , non_empty_iterates_all_elements
         )
  {
    auto const distribution
      { equally_distributed
        ( std::invoke (random_number_of_storages)
        , std::invoke (testing::random::value<core::memory::Size>{})
        )
      };
    auto const used_storages {UsedStorages {make_constructor (distribution)}};

    ASSERT_EQ
      ( distribution.size()
      , std::distance (used_storages.begin(), used_storages.end())
      );

    {
      auto missing {distribution};

      std::ranges::for_each
        ( used_storages
        , [&] (auto used_storage)
          {
            auto const pos {missing.find (used_storage._storage_id)};

            ASSERT_NE (pos, std::end (missing));
            ASSERT_EQ (pos->second, used_storage._range);

            missing.erase (pos);
          }
        );

      ASSERT_TRUE (missing.empty());
    }
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , non_empty_has_full_range
         )
  {
    auto const distribution
      { equally_distributed
        ( std::invoke (random_number_of_storages)
        , std::invoke (testing::random::value<core::memory::Size>{})
        )
      };

    ASSERT_EQ
      ( UsedStorages {make_constructor (distribution)}.range()
      , full_range (distribution)
      );
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , non_empty_lower_bound_returns_smallest_used_storage_that_is_not_smaller_than_an_inner_range
         )
  {
    auto const distribution
      { equally_distributed
        ( std::invoke (random_number_of_storages)
        , std::invoke (testing::random::value<core::memory::Size>{})
        )
      };
    auto const used_storages {UsedStorages (make_constructor (distribution))};
    auto const inner_range {random_inner_range (used_storages.range())};

    auto const is_smaller_than_inner_range
      { [&] (auto range)
        {
          return ! std::less{}
            ( core::memory::begin (inner_range)
            , core::memory::end (range)
            );
        }
      };

    auto const used_storage {used_storages.lower_bound (inner_range)};

    ASSERT_NE (used_storage, std::end (used_storages));
    ASSERT_TRUE (! is_smaller_than_inner_range (used_storage->_range));
    ASSERT_TRUE
      ( std::all_of
        ( std::begin (used_storages)
        , used_storage
        , [&] (auto smaller_used_storage)
          {
            return is_smaller_than_inner_range (smaller_used_storage._range);
          }
        )
      );
  }

  TEST_F (MCSIOVBackendUsedStorageR, emplace_duplicate_into_constructor_throws)
  {
    auto const distribution
      { equally_distributed
        ( std::invoke (random_number_of_storages)
        , std::invoke (testing::random::value<core::memory::Size>{})
        )
      };
    auto const duplicate
      { make_used_storage (testing::random::one_of (distribution))
      };

    testing::require_exception
      ( [&]
        {
          make_constructor (distribution).emplace (duplicate);
        }
      , testing::Assert<UsedStorages::Constructor::Error::Emplace::Duplicate>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.used_storage(), duplicate);
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "UsedStorage::Constructor::Emplace::Duplicate: {}"
                , duplicate
                ).c_str()
              );
          }
        }
      );
  }

  TEST_F (MCSIOVBackendUsedStorageR, construct_with_gap_throws)
  {
    // must be at least 3 to create a gap by removing some "inner"
    auto const number_of_storages
      { std::invoke
        ( RandomNumberOfStorages
          { RandomNumberOfStorages::Min {3}
          , RandomNumberOfStorages::Max {100}
          }
        )
      };
    // size must be not smaller than the number of storages in order
    // to avoid zero size ranges (their removal would _not_ lead to a
    // gap)
    auto const size
      { core::memory::make_size
        ( std::invoke
          ( testing::random::value<std::size_t>
            { testing::random::value<std::size_t>::Min {number_of_storages}
            }
          )
        )
      };
    auto distribution {equally_distributed (number_of_storages, size)};

    ASSERT_GT (distribution.size(), 2);
    ASSERT_TRUE
      ( std::ranges::all_of
        ( distribution
        , [] (auto storage_id_and_range)
          {
            return core::memory::size (storage_id_and_range.second)
              > core::memory::make_size (0)
              ;
          }
        )
      );

    auto const ascending_positions
      { std::invoke
        ( [&]
          {
            auto _positions {std::vector<decltype (distribution)::iterator>{}};

            for ( auto position {std::begin (distribution)}
                ; position != std::end (distribution)
                ; ++position
                )
            {
              _positions.emplace_back (position);
            }

            std::ranges::sort
              ( _positions
              , [] (auto lhs, auto rhs) noexcept
                {
                  return std::less{} (lhs->second, rhs->second);
                }
              );

            return _positions;
          }
        )
      };

    ASSERT_GT (ascending_positions.size(), 2);

    auto const inner_index
      { std::invoke
        ( [&]
          {
            auto const iota
              { std::ranges::iota_view
                { std::size_t {0}
                , ascending_positions.size()
                }
              };
            auto indices
              { std::deque<std::size_t> {std::begin (iota), std::end (iota)}
              };
            indices.pop_front();
            indices.pop_back();

            return testing::random::one_of (indices);
          }
        )
      };

    ASSERT_GT (inner_index, 0);
    ASSERT_LT (inner_index + 1, ascending_positions.size());

    distribution.erase (ascending_positions.at (inner_index));

    testing::require_exception
      ( [&]
        {
          std::ignore = UsedStorages {make_constructor (distribution)};
        }
      , testing::Assert<UsedStorages::Error::Construct::RangesAreNotTouching>
        { [&] (auto const& caught)
          {
            ASSERT_EQ
              ( caught.small()
              , ascending_positions.at (inner_index - 1)->second
              );
            ASSERT_EQ
              ( caught.large()
              , ascending_positions.at (inner_index + 1)->second
              );
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "UsedStorages::Construct::RangesAreNotTouching: "
                  "Ranges must be consecutive. "
                  "There is a gap between {} and {}."
                , caught.small()
                , caught.large()
                ).c_str()
              );
          }
        }
      );
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , append_of_consecutive_ranges_has_ranges_joined
         )
  {
    using RandomSize = testing::random::value<core::memory::Size>;
    auto const max_size
      { std::numeric_limits<core::memory::Size::underlying_type>::max() / 2
      };
    auto used_storages
      { UsedStorages
        { make_constructor
          ( equally_distributed
            ( std::invoke (random_number_of_storages)
            , std::invoke (RandomSize {RandomSize::Max {max_size}})
            )
          )
        }
      };
    auto const to_append
      { UsedStorages
         { make_constructor
           ( shift
             ( equally_distributed
               ( std::invoke (random_number_of_storages)
               , std::invoke (RandomSize {RandomSize::Max {max_size}})
               )
             , core::memory::end (used_storages.range())
             )
           )
         }
      };
    used_storages.append (to_append);

    auto const joined_range
      { core::memory::make_range
        ( core::memory::begin (used_storages.range())
        , core::memory::end (to_append.range())
        )
      };

    ASSERT_EQ (joined_range, used_storages.range());
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , append_of_overlapping_range_throws
         )
  {
    using RandomSize = testing::random::value<core::memory::Size>;
    auto const max_size
      { std::numeric_limits<core::memory::Size::underlying_type>::max() / 2
      };
    auto used_storages
      { UsedStorages
        { make_constructor
          ( equally_distributed
            ( std::invoke (random_number_of_storages)
            , std::invoke (RandomSize {RandomSize::Max {max_size}})
            )
          )
        }
      };
    auto const to_append
      { UsedStorages
         { make_constructor
           ( shift
             ( equally_distributed
               ( std::invoke (random_number_of_storages)
               , std::invoke (RandomSize {RandomSize::Max {max_size}})
               )
             , core::memory::end (used_storages.range())
               - std::invoke
                 ( RandomSize
                   { RandomSize::Min {1}
                   , RandomSize::Max
                     ( size_cast<core::memory::Size::underlying_type>
                       ( core::memory::size (used_storages.range())
                       )
                     )
                   }
                 )
             )
           )
         }
      };

    testing::require_exception
      ( [&]
        {
          used_storages.append (to_append);
        }
      , testing::Assert<UsedStorages::Error::Append::RangesAreNotTouching>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.existing(), used_storages.range());
            ASSERT_EQ (caught.to_append(), to_append.range());
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "UsedStorages::Append::RangesAreNotTouching: "
                  "Ranges must be consecutive. Existing: {}, to append: {}."
                , caught.existing()
                , caught.to_append()
                ).c_str()
              );
          }
        }
      );
  }

  TEST_F ( MCSIOVBackendUsedStorageR
         , append_of_range_with_a_gap_throws
         )
  {
    using RandomSize = testing::random::value<core::memory::Size>;
    auto const limit_max
      { std::numeric_limits<core::memory::Size::underlying_type>::max()
      };
    auto const max_size {limit_max / 3};
    auto used_storages
      { UsedStorages
        { make_constructor
          ( equally_distributed
            ( std::invoke (random_number_of_storages)
            , std::invoke (RandomSize {RandomSize::Max {max_size}})
            )
          )
        }
      };
    auto const size_to_append
      { std::invoke (RandomSize {RandomSize::Max {max_size}})
      };
    auto const to_append
      { UsedStorages
         { make_constructor
           ( shift
             ( equally_distributed
               ( std::invoke (random_number_of_storages)
               , size_to_append
               )
             , core::memory::end (used_storages.range())
               + std::invoke
                 ( RandomSize
                   { RandomSize::Min {1}
                   , RandomSize::Max
                     ( limit_max
                     - offset_cast<core::memory::Size::underlying_type>
                       ( core::memory::end (used_storages.range())
                       )
                     - size_cast<core::memory::Size::underlying_type>
                       ( size_to_append
                       )
                     )
                   }
                 )
             )
           )
         }
      };

    testing::require_exception
      ( [&]
        {
          used_storages.append (to_append);
        }
      , testing::Assert<UsedStorages::Error::Append::RangesAreNotTouching>
        { [&] (auto const& caught)
          {
            ASSERT_EQ (caught.existing(), used_storages.range());
            ASSERT_EQ (caught.to_append(), to_append.range());
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "UsedStorages::Append::RangesAreNotTouching: "
                  "Ranges must be consecutive. Existing: {}, to append: {}."
                , caught.existing()
                , caught.to_append()
                ).c_str()
              );
          }
        }
      );
  }
}
