// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/access.hpp>
#include <mcs/util/read/declare.hpp>

namespace mcs::util::type
{
  // A format-able, readable, de-/serialize-able strong id type for
  // type lists.
  //
  template<typename... Ts>
    struct ID
  {
    constexpr auto operator<=> (ID const&) const noexcept = default;

    // Invoke Runner::template operator()<type_of_id> (args).
    //
    // EXAMPLE:
    //   using IDs = ID<int, A>;
    //   IDs::run
    //     ( IDs::id<int>()
    //     , [&]<typename T> (long v)
    //       {
    //         ASSERT_EQ (typeid (T), typeid (int));
    //         ASSERT_EQ (v, 42L);
    //       }
    //     , 42L
    //     );
    //
    template<typename Runner, typename... Args>
      constexpr auto run (Runner&&, Args&&...) const;

  private:
    template<typename...> friend struct List;

    constexpr explicit ID (std::size_t);

    std::size_t _id;

    MCS_UTIL_FMT_ACCESS();
    MCS_SERIALIZATION_ACCESS();
    MCS_UTIL_READ_ACCESS();
  };
}

namespace fmt
{
  template<typename... Ts>
    MCS_UTIL_FMT_DECLARE (mcs::util::type::ID<Ts...>);
}

namespace mcs::serialization
{
  template<typename... Ts>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::type::ID<Ts...>
      );
}

namespace mcs::util::read
{
  template<typename... Ts>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::type::ID<Ts...>
      );
}

#include "detail/ID.ipp"
