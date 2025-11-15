// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <iterator>
#include <mcs/iov_backend/Location.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <vector>

namespace mcs::iov_backend::provider
{
  struct State;
}

namespace mcs::iov_backend
{
  struct Locations
  {
  private:
    using ContainerOfLocation = std::vector<Location>;

  public:
    struct ConstIterator
    {
      using underlying_iterator = ContainerOfLocation::const_iterator;
      using difference_type = underlying_iterator::difference_type;
      using value_type = Location;

      auto operator*() const -> value_type;
      auto operator++() noexcept -> ConstIterator&;
      auto operator++ (int) noexcept -> ConstIterator;

      auto operator<=> (ConstIterator const&) const noexcept = default;

    private:
      friend struct Locations;
      ConstIterator (underlying_iterator pos) noexcept;
      underlying_iterator _pos;
    };
    static_assert (std::input_iterator<ConstIterator>);

    [[nodiscard]] auto begin() const noexcept -> ConstIterator;
    [[nodiscard]] auto end() const noexcept -> ConstIterator;

  private:
    friend struct provider::State;
    Locations (ContainerOfLocation);

    ContainerOfLocation _locations;

    MCS_SERIALIZATION_ACCESS();
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( mcs::iov_backend::Locations
      );
}
