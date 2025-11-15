// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/type/Index.hpp>
#include <mcs/util/type/Size.hpp>
#include <mcs/util/type/detail/ParseIDAndRun.hpp>
#include <type_traits>
#include <utility>

namespace mcs::util::type
{
  template<typename... Ts>
    constexpr auto List<Ts...>::size() noexcept -> std::size_t
  {
    return Size<Ts...>::value;
  }

  template<typename... Ts>
    template<typename T>
      constexpr auto List<Ts...>::contains() noexcept -> bool
  {
    return (std::is_same_v<T, Ts> || ...);
  }

  template<typename... Ts>
    template<typename T>
      constexpr auto List<Ts...>::id() noexcept -> ID
  {
    static_assert (List<Ts...>::contains<T>());

    return ID {Index<T, Ts...>()};
  }
}

namespace mcs::util::type
{
  template<typename... Ts>
    template<typename Runner, typename... Args>
      constexpr auto List<Ts...>::run
        ( ID id
        , Runner&& runner
        , Args&&... args
        )
  {
    return id.template run<Runner, Args...>
      ( std::forward<Runner> (runner)
      , std::forward<Args> (args)...
      );
  }
}

namespace mcs::util::type
{
  template<typename... Ts>
    template< template<typename> typename Parse
            , typename Char
            , typename Runner
            , typename... Args
            >
      auto List<Ts...>::parse_id_and_run
        ( util::read::State<Char>& state
        , Runner&& runner
        , Args&&... args
        )
  {
    return detail::ParseIDAndRun<Parse, Ts...>{}
      ( state
      , std::forward<Runner> (runner)
      , std::forward<Args> (args)...
      );
  }

  template<typename... Ts>
    template< template<typename> typename Parse
            , typename Runner
            , typename... Args
            >
      auto List<Ts...>::parse_id_and_run
        ( std::string input
        , Runner&& runner
        , Args&&... args
        )
  {
    auto state {util::read::make_state (input)};

    return List<Ts...>::parse_id_and_run<Parse>
      ( state
      , std::forward<Runner> (runner)
      , std::forward<Args> (args)...
      );
  }
}
