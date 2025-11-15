// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <mcs/util/FMT/STD/variant.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/STD/variant.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/read.hpp>
#include <utility>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::util::ASIO::Connectable<asio::ip::tcp>)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    ( connectable
    , ctx
    , mcs::util::ASIO::Connectable<asio::ip::tcp>
    )
  {
    return fmt::format_to
      ( ctx.out()
      , "ip::tcp {}"
      , std::make_tuple (connectable.address_or_hostname, connectable.port)
      );
  }

  MCS_UTIL_FMT_DEFINE_PARSE
    ( ctx
    , mcs::util::ASIO::Connectable<asio::ip::tcp>::Address
    )
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    (address, ctx, mcs::util::ASIO::Connectable<asio::ip::tcp>::Address)
  {
    return fmt::format_to (ctx.out(), "Address {}", address.address_string);
  }

  MCS_UTIL_FMT_DEFINE_PARSE
    ( ctx
    , mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname
    )
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    (hostname, ctx, mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname)
  {
    return fmt::format_to (ctx.out(), "Hostname {}", hostname.hostname);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, util::ASIO::Connectable<asio::ip::tcp>)
  {
    prefix (state, "ip::tcp");

    using Connectable = util::ASIO::Connectable<asio::ip::tcp>;

    if (maybe<std::tuple<>> (state))
    {
      return Connectable {asio::ip::tcp::endpoint{}};
    }

    return std::make_from_tuple<Connectable>
      ( parse< std::tuple
               < decltype (std::declval<Connectable>().address_or_hostname)
               , decltype (std::declval<Connectable>().port)
               >
             > (state)
      );
  }

  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, util::ASIO::Connectable<asio::ip::tcp>::Address)
  {
    prefix (state, "Address");

    using Addr = util::ASIO::Connectable<asio::ip::tcp>::Address;

    return Addr
      { parse<decltype (std::declval<Addr>().address_string)> (state)
      };
  }

  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, util::ASIO::Connectable<asio::ip::tcp>::Hostname)
  {
    prefix (state, "Hostname");

    using Hostname = util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    return Hostname
      { parse<decltype (std::declval<Hostname>().hostname)> (state)
      };
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE
    ( ctx
    , mcs::util::ASIO::Connectable<asio::local::stream_protocol>
    )
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    ( connectable
    , ctx
    , mcs::util::ASIO::Connectable<asio::local::stream_protocol>
    )
  {
    return fmt::format_to
      ( ctx.out()
      , "local::stream_protocol {}"
      , std::make_tuple (connectable.path)
      );
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, util::ASIO::Connectable<asio::local::stream_protocol>)
  {
    prefix (state, "local::stream_protocol");

    using Connectable = util::ASIO::Connectable<asio::local::stream_protocol>;

    return std::make_from_tuple<Connectable>
      (parse<std::tuple< decltype (std::declval<Connectable>().path)
                       >
            > (state)
      );
  }
}

namespace mcs::util::ASIO
{
  template<typename Runner, typename... Args>
    auto run (AnyConnectable connectable, Runner&& runner, Args&&... args)
  {
    return std::visit
      ( util::overloaded
        ( [&] (Connectable<asio::ip::tcp> _connectable)
          {
            return std::forward<Runner> (runner)
              . template operator()<asio::ip::tcp>
                  ( _connectable
                  , std::forward<Args> (args)...
                  );
          }
        , [&] (Connectable<asio::local::stream_protocol> _connectable)
          {
            return std::forward<Runner> (runner)
              . template operator()<asio::local::stream_protocol>
                  ( _connectable
                  , std::forward<Args> (args)...
                  );
          }
        )
      , connectable
      );
  }
}

namespace mcs::util::ASIO
{
  template<typename Endpoint>
    auto make_connectable (Endpoint const& endpoint)
  {
    if constexpr (std::is_same_v<Endpoint, asio::ip::tcp::endpoint>)
    {
      return Connectable<asio::ip::tcp> (endpoint);
    }

    if constexpr (std::is_same_v<Endpoint, asio::local::stream_protocol::endpoint>)
    {
      return Connectable<asio::local::stream_protocol> (endpoint);
    }
  }
}
