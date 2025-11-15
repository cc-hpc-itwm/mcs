// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
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
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      (oa, id, util::type::ID<Ts...>)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, id, _id);

    return oa;
  }

  template<typename... Ts>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      (ia, util::type::ID<Ts...>)
  {
    using ID = util::type::ID<Ts...>;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _id, ID);

    return ID {_id};
  }
}

namespace fmt
{
  template<typename... Ts>
    MCS_UTIL_FMT_DEFINE_PARSE (context, mcs::util::type::ID<Ts...>)
  {
    return context.begin();
  }

  template<typename... Ts>
    MCS_UTIL_FMT_DEFINE_FORMAT (id, context, mcs::util::type::ID<Ts...>)
  {
    return fmt::format_to (context.out(), "type_id_{}", id._id);
  }
}

namespace mcs::util::read
{
  template<typename... Ts>
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
      ( state
      , util::type::ID<Ts...>
      )
  {
    using ID = util::type::ID<Ts...>;

    prefix (state, "type_id_");

    auto const id {parse<decltype (ID::_id)> (state)};

    return ID {id};
  }
}
