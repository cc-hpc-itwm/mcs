// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/hash/define.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <utility>

namespace mcs::util::ASIO
{
  Connectable<asio::ip::tcp>::Connectable
    ( asio::ip::tcp::endpoint const& endpoint
    )
      : address_or_hostname
        { endpoint.address().is_unspecified()
        ? decltype (address_or_hostname)
            {Hostname {util::string {util::syscall::hostname()}}}
        : decltype (address_or_hostname)
            {Address {util::string {endpoint.address().to_string()}}}
        }
      , port {endpoint.port()}
  {}

  Connectable<asio::ip::tcp>::Connectable
    ( decltype (address_or_hostname) address_or_hostname_
    , decltype (port) port_
    )
      : address_or_hostname {address_or_hostname_}
      , port {port_}
  {}
}

namespace mcs::util::ASIO
{
  Connectable<asio::local::stream_protocol>::Connectable
    ( asio::local::stream_protocol::endpoint const& endpoint
    )
      : path {endpoint.path()}
  {}

  Connectable<asio::local::stream_protocol>::Connectable
    ( decltype (path) path_
    )
      : path {path_}
  {}
}

namespace std
{
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER
    (address_string, mcs::util::ASIO::Connectable<asio::ip::tcp>::Address);
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER
    (hostname, mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname);
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER
    (address_or_hostname, mcs::util::ASIO::Connectable<asio::ip::tcp>);
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER
    (path, mcs::util::ASIO::Connectable<asio::local::stream_protocol>);
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, address, util::ASIO::Connectable<asio::ip::tcp>::Address)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, address, address_string);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, util::ASIO::Connectable<asio::ip::tcp>::Address)
  {
    using Address = util::ASIO::Connectable<asio::ip::tcp>::Address;

    MCS_SERIALIZATION_LOAD_FIELD (ia, address_string, Address);

    return Address {std::move (address_string)};
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, hostname, util::ASIO::Connectable<asio::ip::tcp>::Hostname)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, hostname, hostname);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, util::ASIO::Connectable<asio::ip::tcp>::Hostname)
  {
    using Hostname = util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    MCS_SERIALIZATION_LOAD_FIELD (ia, hostname, Hostname);

    return Hostname {std::move (hostname)};
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, connectable, util::ASIO::Connectable<asio::ip::tcp>)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, connectable, address_or_hostname);
    MCS_SERIALIZATION_SAVE_FIELD (oa, connectable, port);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, util::ASIO::Connectable<asio::ip::tcp>)
  {
    using Connectable = util::ASIO::Connectable<asio::ip::tcp>;

    MCS_SERIALIZATION_LOAD_FIELD (ia, address_or_hostname, Connectable);
    MCS_SERIALIZATION_LOAD_FIELD (ia, port, Connectable);

    return Connectable
      { std::move (address_or_hostname)
      , port
      };
  }
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, connectable, util::ASIO::Connectable<asio::local::stream_protocol>)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, connectable, path);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, util::ASIO::Connectable<asio::local::stream_protocol>)
  {
    using Connectable = util::ASIO::Connectable<asio::local::stream_protocol>;

    MCS_SERIALIZATION_LOAD_FIELD (ia, path, Connectable);

    return Connectable
      { std::move (path)
      };
  }
}
