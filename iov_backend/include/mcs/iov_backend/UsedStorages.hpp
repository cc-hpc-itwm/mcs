// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/UsedStorage.hpp>
#include <mcs/serialization/STD/set.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <set>
#include <type_traits>

namespace mcs::iov_backend
{
  // The distribution of a collection across some storages.
  //
  struct UsedStorages
  {
    struct Error
    {
      struct Construct
      {
        struct RangesAreNotTouching : public mcs::Error
        {
          [[nodiscard]] constexpr auto small
            (
            ) const noexcept -> core::memory::Range
            ;
          [[nodiscard]] constexpr auto large
            (
            ) const noexcept -> core::memory::Range
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (RangesAreNotTouching);

        private:
          friend struct UsedStorages;
          [[nodiscard]] RangesAreNotTouching
            ( core::memory::Range
            , core::memory::Range
            );
          core::memory::Range _small;
          core::memory::Range _large;
        };
      };

      struct Append
      {
        struct RangesAreNotTouching : public mcs::Error
        {
          [[nodiscard]] constexpr auto existing
            (
            ) const noexcept -> core::memory::Range
            ;
          [[nodiscard]] constexpr auto to_append
            (
            ) const noexcept -> core::memory::Range
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (RangesAreNotTouching);

        private:
          friend struct UsedStorages;
          [[nodiscard]] RangesAreNotTouching
            ( core::memory::Range
            , core::memory::Range
            );
          core::memory::Range _existing;
          core::memory::Range _to_append;
        };
      };

      struct Range
      {
        struct Empty : public mcs::Error
        {
          MCS_ERROR_COPY_MOVE_DEFAULT (Empty);

        private:
          friend struct UsedStorages;
          [[nodiscard]] Empty();
        };
      };
    };

    struct Compare
    {
      using is_transparent = std::true_type;

      // used to order the set
      [[nodiscard]] constexpr auto operator()
        ( UsedStorage const&
        , UsedStorage const&
        ) const noexcept -> bool
        ;
      // used to lower_bound
      [[nodiscard]] constexpr auto operator()
        ( UsedStorage const&
        , core::memory::Range const&
        ) const noexcept -> bool
        ;
    };

    using Container = std::set<UsedStorage, Compare>;

    struct Constructor
    {
      template<typename... Args>
        requires (std::is_constructible_v<UsedStorage, Args...>)
        auto emplace (Args&&...);

      struct Error
      {
        struct Emplace
        {
          struct Duplicate : public mcs::Error
          {
            [[nodiscard]] auto used_storage() const -> UsedStorage;

            MCS_ERROR_COPY_MOVE_DEFAULT (Duplicate);

          private:
            friend struct Constructor;
            [[nodiscard]] Duplicate (UsedStorage);
            UsedStorage _used_storage;
          };
        };
      };

    private:
      friend struct UsedStorages;
      Container _container;
    };
    UsedStorages (Constructor);

    auto append (UsedStorages) -> void;

    struct ConstIterator
    {
      using difference_type = std::ptrdiff_t;
      using value_type = UsedStorage;

      auto operator->() const noexcept -> value_type const*;
      auto operator*() const noexcept -> value_type const&;
      auto operator++() noexcept -> ConstIterator&;
      auto operator++ (int) noexcept -> ConstIterator;

      auto operator!= (ConstIterator const&) const noexcept -> bool;
      auto operator== (ConstIterator const&) const noexcept -> bool;

      ConstIterator() noexcept = default;

    private:
      friend struct UsedStorages;
      using underlying_iterator = Container::const_iterator;
      ConstIterator (underlying_iterator pos, underlying_iterator end) noexcept;
      underlying_iterator _pos;
      underlying_iterator _end;
    };
    static_assert (std::forward_iterator<ConstIterator>);

    [[nodiscard]] auto begin() const noexcept -> ConstIterator;
    [[nodiscard]] auto end() const noexcept -> ConstIterator;

    // Returns: The iterator to the smallest storage that is not
    // smaller than the given range, that is the smallest storage such
    // that the begin of the given range is smaller than the end of
    // the returned storage.
    //
    // EXAMPLE:
    //
    // storages      [---)
    //               ^   [-------)
    //               :     ^     [----)
    //               :     :     ^    [-------)END
    //               :     :     :             ^    ^
    // range       [---    :     :             :    :
    //                     [---  :             :    :
    //                           [---          :    :
    //                                        [---  :
    //                                              [---
    //
    [[nodiscard]] auto lower_bound
      ( core::memory::Range
      ) const noexcept -> ConstIterator
      ;

    // Requires: !empty()
    // O(1)
    [[nodiscard]] auto range() const -> core::memory::Range;
    [[nodiscard]] auto empty() const noexcept -> bool;

  private:
    Container _container;

    MCS_SERIALIZATION_ACCESS();
    explicit UsedStorages (Container) noexcept;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::UsedStorages
      )
    ;
}

#include "detail/UsedStorages.ipp"
