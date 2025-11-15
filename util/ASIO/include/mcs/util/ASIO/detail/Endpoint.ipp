// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/maybe.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/string.hpp>
#include <utility>

namespace mcs::util::ASIO
{
  template<typename Runner, typename... Args>
    auto run (Endpoint endpoint, Runner&& runner, Args&&... args)
  {
    return std::visit
      ( util::overloaded
        ( [&] (asio::ip::tcp::endpoint ep)
          {
            return runner.template operator()<asio::ip::tcp>
              ( ep
              , std::forward<Args> (args)...
              );
          }
        , [&] (asio::local::stream_protocol::endpoint ep)
          {
            return runner.template operator()<asio::local::stream_protocol>
              ( ep
              , std::forward<Args> (args)...
              );
          }
        )
      , endpoint
      );
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    ( state
    , asio::ip::tcp::endpoint
    )
  {
    using EP = asio::ip::tcp::endpoint;
    using Port = decltype (std::declval<EP>().port());

    prefix (state, "ip::tcp");

    if (maybe<std::tuple<>> (state))
    {
      return asio::ip::tcp::endpoint{};
    }
    else
    {
      auto [address, port] {parse<std::tuple<string, Port>> (state)};

      return asio::ip::tcp::endpoint
        ( asio::ip::make_address (address)
        , port
        );
    }
  }

  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, asio::local::stream_protocol::endpoint)
  {
    prefix (state, "local::stream_protocol");

    auto [path] {parse<std::tuple<string>> (state)};

    return asio::local::stream_protocol::endpoint {path};
  }
}
