// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/iov_backend/provider/distribution/AsEqualAsPossible.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/random/Test.hpp>
#include <numeric>
#include <ranges>

namespace mcs::iov_backend::provider::distribution
{
  namespace
  {
    struct IOVBackendProviderDistributionAsEqualAsPossibleR
      : public testing::random::Test
    {
      using RandomSize = testing::random::value<core::memory::Size>;

      struct Capacity
      {
        bool want_more {false};
        core::storage::MaxSize max_size;
      };

      template<typename MakeCapacity>
        auto generate_capacities (MakeCapacity make_capacity)
      {
        auto capacities
          { std::priority_queue<AsEqualAsPossible::StorageIDAndCapacity>{}
          };

        auto next_id {storage::ID{}};

      MAKE_CAPACITY:
        auto const [want_more, capacity] {make_capacity()};

        capacities.emplace (next_id, capacity);

        ++next_id;

        if (want_more)
        {
          goto MAKE_CAPACITY;
        }

        return capacities;
      }

      template<typename... Limits>
        auto make_limits (Limits... limits)
      {
        auto capacities
          { std::priority_queue<AsEqualAsPossible::StorageIDAndCapacity>{}
          };

        auto next_id {storage::ID{}};
        auto const add_storage
          { [&] (auto l)
            {
              capacities.emplace
                ( next_id
                , core::storage::MaxSize::Limit {core::memory::make_size (l)}
                );

              ++next_id;
            }
          };

        (add_storage (limits), ...);

        return capacities;
      }
    };

    auto assigned_ranges_respect_capacities
      ( auto assigned_ranges
      , auto capacities
      )
    {
      while (!capacities.empty())
      {
        auto const [storage_id, capacity] {capacities.top()};
        capacities.pop();

        if (core::memory::size (assigned_ranges.at (storage_id)) > capacity)
        {
          return false;
        }

        assigned_ranges.erase (storage_id);
      }

      return assigned_ranges.empty();
    }

    auto all_assigned_ranges_use_their_complete_storage
      ( auto assigned_ranges
      , auto capacities
      )
    {
      while (!capacities.empty())
      {
        auto const [storage_id, capacity] {capacities.top()};
        capacities.pop();

        if (  core::memory::size (assigned_ranges.at (storage_id))
           != capacity.limit()
           )
        {
          return false;
        }

        assigned_ranges.erase (storage_id);
      }

      return assigned_ranges.empty();
    }

    auto assigned_ranges_are_a_touching_partition
      ( auto assigned_ranges
      , auto size
      )
    {
      auto const ranges {std::views::values (assigned_ranges)};

      auto const sum_of_sizes_equals_the_total_size
        { std::invoke
          ( [&]
            {
              return size
                == std::accumulate
                   ( std::begin (ranges), std::end (ranges)
                   , core::memory::make_size (0)
                   , [] (auto sum, auto range)
                     {
                       return sum + core::memory::size (range);
                     }
                   )
                ;
            }
          )
        };

      auto const ranges_ascending
        { std::invoke
          ( [&]
            {
              auto _ranges_ascending
                { std::vector<core::memory::Range>
                    { std::begin (ranges)
                    , std::end (ranges)
                    }
                };
              std::ranges::sort (_ranges_ascending);
              return _ranges_ascending;
            }
          )
        };

      auto const ranges_cover_the_full_range
        { std::invoke
          ( [&]
            {
              auto const full_range
                { core::memory::make_range
                  ( core::memory::make_offset (0)
                  , size
                  )
                };

              return
                (  core::memory::begin (ranges_ascending.front())
                == core::memory::begin (full_range)
                )
                &&
                (   core::memory::end (ranges_ascending.back())
                ==  core::memory::end (full_range)
                );
            }
          )
        };

      auto const ranges_are_touching
        { std::invoke
          ( [&]
            {
              if (ranges_ascending.size() < 2)
              {
                return true;
              }

              for ( auto range {std::begin (ranges_ascending)}
                  ; std::next (range) != std::end (ranges_ascending)
                  ; ++range
                  )
              {
                if (  core::memory::end (*range)
                   != core::memory::begin (*std::next (range))
                   )
                {
                  return false;
                }
              }

              return true;
            }
          )
        };

      return sum_of_sizes_equals_the_total_size
        && ranges_cover_the_full_range
        && ranges_are_touching
        ;
    }

    auto assigned_ranges_are_a_capacity_respecting_touching_partition
      ( auto assigned_ranges
      , auto capacities
      , auto size
      )
    {
      return assigned_ranges_respect_capacities (assigned_ranges, capacities)
        && assigned_ranges_are_a_touching_partition (assigned_ranges, size)
        ;
    }

    auto size_of_assigned_ranges_differ_pairwise_by_at_most_one
      (auto assigned_ranges)
    {
      auto const ranges {std::views::values (assigned_ranges)};

      for (auto const rx : ranges)
      {
        for (auto const ry : ranges)
        {
          auto const sx {size_cast<std::size_t> (core::memory::size (rx))};
          auto const sy {size_cast<std::size_t> (core::memory::size (ry))};
          auto const range_diff {std::max (sx, sy) - std::min (sx, sy)};

          if (range_diff > 1UL)
          {
            return false;
          }
        }
      }

      return true;
    }
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , the_demo_example_works
         )
  {
    auto const capacities {make_limits (9, 11, 10, 10)};

    auto const size {core::memory::make_size (40)};

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( all_assigned_ranges_use_their_complete_storage
        ( assigned_ranges
        , capacities
        )
      );
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , the_12_5_example_works
         )
  {
    auto const capacities {make_limits (3, 2, 3, 2, 2)};

    auto const size {core::memory::make_size (12)};

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( all_assigned_ranges_use_their_complete_storage
        ( assigned_ranges
        , capacities
        )
      );
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , the_2_4_example_works
         )
  {
    auto const capacities {make_limits (1, 0, 1, 0)};

    auto const size {core::memory::make_size (2)};

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( all_assigned_ranges_use_their_complete_storage
        ( assigned_ranges
        , capacities
        )
      );
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , equal_distribution_if_all_storages_have_unlimited_capacity
         )
  {
    auto const capacities
      { generate_capacities
        ( [ number_of_storages
            { std::invoke (testing::random::value<int> {1, 1'000})
            }
          ]() mutable
          {
            return Capacity
              { --number_of_storages > 0
              , core::storage::MaxSize::Unlimited{}
              };
          }
        )
      };

    auto const size
      { std::invoke
        ( RandomSize
          { RandomSize::Min {0UL}
          , RandomSize::Max {32UL << 30UL}
          }
        )
      };

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( size_of_assigned_ranges_differ_pairwise_by_at_most_one
         ( assigned_ranges
         )
      );
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , equal_distribution_if_all_storages_have_enough_capacity
         )
  {
    auto const size
      { std::invoke
        ( RandomSize
          { RandomSize::Min {0UL}
          , RandomSize::Max {32UL << 30UL}
          }
        )
      };

    auto const capacities
      { std::invoke
        ( [&]
          {
            auto number_of_storages
              { std::invoke (testing::random::value<std::size_t> {1UL, 1'000UL})
              };
            auto const size_at_least
              { util::divru (size_cast<std::size_t> (size), number_of_storages)
              };
            auto random_size
              { RandomSize
                { RandomSize::Min {size_at_least}
                  , RandomSize::Max {size_cast<std::size_t> (size)}
                }
              };

            return generate_capacities
              ( [&]
                {
                  return Capacity
                    { --number_of_storages > 0
                    , core::storage::MaxSize::Limit {std::invoke (random_size)}
                    };
                }
              );
          }
        )
      };

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( size_of_assigned_ranges_differ_pairwise_by_at_most_one
         ( assigned_ranges
         )
      );
  }

  TEST_F ( IOVBackendProviderDistributionAsEqualAsPossibleR
         , small_storages_are_filled
         )
  {
    auto const average_size_per_storage
      { std::invoke
        ( testing::random::value<std::size_t> {1UL << 10UL, 1UL << 20UL}
        )
      };
    auto const number_of_storages
      { std::invoke (testing::random::value<std::size_t> {1UL, 1'000UL})
      };
    auto const size
      { core::memory::make_size (average_size_per_storage * number_of_storages)
      };

    auto const capacities
      { std::invoke
        ( [&]
          {
            auto not_yet_distributed {size};
            auto storage {0UL};
            auto random_small_size
              { RandomSize
                { RandomSize::Min {0UL}
                , RandomSize::Max {average_size_per_storage}
                }
              };

            return generate_capacities
              ( [&]
                {
                  if (++storage == number_of_storages)
                  {
                    return Capacity
                      { false
                      , core::storage::MaxSize::Limit {not_yet_distributed}
                      };
                  }
                  else
                  {
                    auto const small_size {std::invoke (random_small_size)};
                    not_yet_distributed -= small_size;

                    return Capacity
                      { not_yet_distributed > core::memory::make_size (0)
                      , core::storage::MaxSize::Limit {small_size}
                      };
                  }
                }
              );
          }
        )
      };

    auto const assigned_ranges {AsEqualAsPossible{} (size, capacities)};

    ASSERT_TRUE
      ( assigned_ranges_are_a_capacity_respecting_touching_partition
        ( assigned_ranges
        , capacities
        , size
        )
      );

    ASSERT_TRUE
      ( all_assigned_ranges_use_their_complete_storage
        ( assigned_ranges
        , capacities
        )
      );
  }
}
