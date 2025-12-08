// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/define.hpp>
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
    MCS_UTIL_FMT_DEFINE_PARSE
      ( ctx
      , mcs::core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
  {
    return ctx.begin();
  }
  template<mcs::util::ASIO::is_protocol Protocol>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( connection_information
      , ctx
      , mcs::core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
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
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
      ( state
      , core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      )
  {
    prefix (state, "ConnectionInformation");

    using ConnectionInformation
      = core::transport::implementation::libfabric::provider
        ::ConnectionInformation<Protocol>
      ;

    return std::make_from_tuple<ConnectionInformation>
      ( parse< std::tuple
               < decltype (std::declval<ConnectionInformation>().control)
               , decltype (std::declval<ConnectionInformation>().transport)
               >
             > (state)
      );
  }
}
