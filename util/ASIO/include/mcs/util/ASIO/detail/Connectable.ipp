// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <mcs/util/FMT/STD/variant.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/STD/variant.hpp>
#include <mcs/util/read/read.hpp>
#include <utility>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>>::format
      ( mcs::util::ASIO::Connectable<asio::ip::tcp> const& connectable
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "ip::tcp {}"
      , std::make_tuple (connectable.address_or_hostname, connectable.port)
      );
  }

  template<typename ParseContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Address>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Address>::format
      ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Address const& address
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "Address {}", address.address_string);
  }

  template<typename ParseContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname>::format
      ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname const& hostname
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "Hostname {}", hostname.hostname);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<util::ASIO::Connectable<asio::ip::tcp>>::read
      ( State<Char>& state
      ) -> util::ASIO::Connectable<asio::ip::tcp>
  {
    prefix (state, "ip::tcp");

    using Connectable = util::ASIO::Connectable<asio::ip::tcp>;

    if (maybe<std::tuple<>> (state))
    {
      return Connectable {asio::ip::tcp::endpoint{}};
    }

    return std::make_from_tuple<Connectable>
      ( parse< std::tuple
               < decltype (Connectable::address_or_hostname)
               , decltype (Connectable::port)
               >
             > (state)
      );
  }

  template<typename Char>
    auto Read<util::ASIO::Connectable<asio::ip::tcp>::Address>::read
      ( State<Char>& state
      ) -> util::ASIO::Connectable<asio::ip::tcp>::Address
  {
    prefix (state, "Address");

    using Addr = util::ASIO::Connectable<asio::ip::tcp>::Address;

    return Addr
      { parse<decltype (Addr::address_string)> (state)
      };
  }

  template<typename Char>
    auto Read<util::ASIO::Connectable<asio::ip::tcp>::Hostname>::read
      ( State<Char>& state
      ) -> util::ASIO::Connectable<asio::ip::tcp>::Hostname
  {
    prefix (state, "Hostname");

    using Hostname = util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    return Hostname
      { parse<decltype (Hostname::hostname)> (state)
      };
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::local::stream_protocol>>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::ASIO::Connectable<asio::local::stream_protocol>>::format
      ( mcs::util::ASIO::Connectable<asio::local::stream_protocol> const& connectable
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
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
  template<typename Char>
    auto Read<util::ASIO::Connectable<asio::local::stream_protocol>>::read
      ( State<Char>& state
      ) -> util::ASIO::Connectable<asio::local::stream_protocol>
  {
    prefix (state, "local::stream_protocol");

    using Connectable = util::ASIO::Connectable<asio::local::stream_protocol>;

    return std::make_from_tuple<Connectable>
      (parse<std::tuple< decltype (Connectable::path)
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
