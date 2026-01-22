// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/type/detail/Run.hpp>

namespace mcs::util::type
{
  template<typename... Ts>
    constexpr ID<Ts...>::ID (std::size_t id)
      : _id {id}
  {}
}

namespace mcs::util::type
{
  template<typename... Ts>
    template<typename Runner, typename... Args>
      constexpr auto ID<Ts...>::run
        ( Runner&& runner
        , Args&&... args
        ) const
  {
    return detail::Run<Ts...>{}
      ( _id
      , std::forward<Runner> (runner)
      , std::forward<Args> (args)...
      );
  }
}

namespace mcs::serialization
{
  template<typename... Ts>
    auto Implementation<util::type::ID<Ts...>>::output
      ( OArchive& oa
      , util::type::ID<Ts...> const& id
      ) -> OArchive&
  {
    save (oa, id._id);

    return oa;
  }

  template<typename... Ts>
    auto Implementation<util::type::ID<Ts...>>::input
      ( IArchive& ia
      ) -> util::type::ID<Ts...>
  {
    using ID = util::type::ID<Ts...>;

    auto _id {load<decltype (ID::_id)> (ia)};

    return ID {_id};
  }
}

namespace fmt
{
  template<typename... Ts>
    template<typename ParseContext>
      constexpr auto formatter<mcs::util::type::ID<Ts...>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename... Ts>
    template<typename FormatContext>
      constexpr auto formatter<mcs::util::type::ID<Ts...>>::format
        ( mcs::util::type::ID<Ts...> const& id
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "type_id_{}", id._id);
  }
}

namespace mcs::util::read
{
  template<typename... Ts>
    template<typename Char>
      auto Read<util::type::ID<Ts...>>::read
        ( State<Char>& state
        ) -> util::type::ID<Ts...>
  {
    using ID = util::type::ID<Ts...>;

    prefix (state, "type_id_");

    auto const id {parse<decltype (ID::_id)> (state)};

    return ID {id};
  }
}
