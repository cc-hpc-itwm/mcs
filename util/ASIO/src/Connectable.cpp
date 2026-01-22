// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
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
  auto hash<mcs::util::ASIO::Connectable<asio::ip::tcp>::Address>::operator()
    ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Address const& x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x.address_string);
  }

  auto hash<mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname>::operator()
    ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname const& x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x.hostname);
  }

  auto hash<mcs::util::ASIO::Connectable<asio::ip::tcp>>::operator()
    ( mcs::util::ASIO::Connectable<asio::ip::tcp> const& x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x.address_or_hostname);
  }

  auto hash<mcs::util::ASIO::Connectable<asio::local::stream_protocol>>::operator()
    ( mcs::util::ASIO::Connectable<asio::local::stream_protocol> const& x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x.path);
  }
}

namespace mcs::serialization
{
  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>::Address>::output
    ( OArchive& oa
    , util::ASIO::Connectable<asio::ip::tcp>::Address const& address
    ) -> OArchive&
  {
    save (oa, address.address_string);

    return oa;
  }
  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>::Address>::input
    ( IArchive& ia
    ) -> util::ASIO::Connectable<asio::ip::tcp>::Address
  {
    using Address = util::ASIO::Connectable<asio::ip::tcp>::Address;

    auto address_string {load<decltype (Address::address_string)> (ia)};

    return Address {std::move (address_string)};
  }

  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>::Hostname>::output
    ( OArchive& oa
    , util::ASIO::Connectable<asio::ip::tcp>::Hostname const& hostname
    ) -> OArchive&
  {
    save (oa, hostname.hostname);

    return oa;
  }
  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>::Hostname>::input
    ( IArchive& ia
    ) -> util::ASIO::Connectable<asio::ip::tcp>::Hostname
  {
    using Hostname = util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    auto hostname {load<decltype (Hostname::hostname)> (ia)};

    return Hostname {std::move (hostname)};
  }

  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>>::output
    ( OArchive& oa
    , util::ASIO::Connectable<asio::ip::tcp> const& connectable
    ) -> OArchive&
  {
    save (oa, connectable.address_or_hostname);
    save (oa, connectable.port);

    return oa;
  }
  auto Implementation<util::ASIO::Connectable<asio::ip::tcp>>::input
    ( IArchive& ia
    ) -> util::ASIO::Connectable<asio::ip::tcp>
  {
    using Connectable = util::ASIO::Connectable<asio::ip::tcp>;

    auto address_or_hostname {load<decltype (Connectable::address_or_hostname)> (ia)};
    auto port {load<decltype (Connectable::port)> (ia)};

    return Connectable
      { std::move (address_or_hostname)
      , port
      };
  }
}

namespace mcs::serialization
{
  auto Implementation<util::ASIO::Connectable<asio::local::stream_protocol>>::output
    ( OArchive& oa
    , util::ASIO::Connectable<asio::local::stream_protocol> const& connectable
    ) -> OArchive&
  {
    save (oa, connectable.path);

    return oa;
  }
  auto Implementation<util::ASIO::Connectable<asio::local::stream_protocol>>::input
    ( IArchive& ia
    ) -> util::ASIO::Connectable<asio::local::stream_protocol>
  {
    using Connectable = util::ASIO::Connectable<asio::local::stream_protocol>;

    auto path {load<decltype (Connectable::path)> (ia)};

    return Connectable
      { std::move (path)
      };
  }
}
