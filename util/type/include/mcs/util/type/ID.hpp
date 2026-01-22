// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <fmt/base.h>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

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

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
  };
}

namespace fmt
{
  template<typename... Ts>
    struct formatter<mcs::util::type::ID<Ts...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::type::ID<Ts...> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<typename... Ts>
    struct Implementation<util::type::ID<Ts...>>
  {
    using Type = util::type::ID<Ts...>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<typename... Ts>
    struct Read<util::type::ID<Ts...>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> util::type::ID<Ts...>
        ;
  };
}

#include "detail/ID.ipp"
