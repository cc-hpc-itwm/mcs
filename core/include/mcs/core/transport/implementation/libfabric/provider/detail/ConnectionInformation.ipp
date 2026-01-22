// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/Read.hpp>
#include <tuple>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::provider
{
  template<typename Runner, typename... Args>
    auto run
      ( AnyConnectionInformation connection_information
      , Runner&& runner
      , Args&&... args
      )
  {
    return std::visit
      ( util::overloaded
        ( [&] ( ConnectionInformation<asio::ip::tcp>
                  _connection_information
              )
          {
            return std::forward<Runner> (runner)
              . template operator()<asio::ip::tcp>
                  ( _connection_information
                  , std::forward<Args> (args)...
                  );
          }
        , [&] ( ConnectionInformation<asio::local::stream_protocol>
                   _connection_information
              )
          {
            return std::forward<Runner> (runner)
              . template operator()<asio::local::stream_protocol>
                  ( _connection_information
                  , std::forward<Args> (args)...
                  );
          }
        )
      , connection_information
      );
  }
}

namespace fmt
{
  template<mcs::util::ASIO::is_protocol Protocol>
    template<typename ParseContext>
      constexpr auto formatter
        < mcs::core::transport::implementation::libfabric::provider
          ::ConnectionInformation<Protocol>
        >::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<mcs::util::ASIO::is_protocol Protocol>
    template<typename FormatContext>
      constexpr auto formatter
        < mcs::core::transport::implementation::libfabric::provider
          ::ConnectionInformation<Protocol>
        >::format
          ( mcs::core::transport::implementation::libfabric::provider
              ::ConnectionInformation<Protocol> const& connection_information
          , FormatContext& ctx
          ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "ConnectionInformation ({}, {})"
      , connection_information.control
      , connection_information.transport
      );
  }
}

namespace mcs::util::read
{
  template<util::ASIO::is_protocol Protocol>
    template<typename Char>
      auto Read<core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>>::read
        ( State<Char>& state
        ) -> core::transport::implementation::libfabric::provider
          ::ConnectionInformation<Protocol>
  {
    prefix (state, "ConnectionInformation");

    using ConnectionInformation
      = core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      ;

    return std::make_from_tuple<ConnectionInformation>
      ( parse< std::tuple
               < decltype (ConnectionInformation::control)
               , decltype (ConnectionInformation::transport)
               >
             > (state)
      );
  }
}
