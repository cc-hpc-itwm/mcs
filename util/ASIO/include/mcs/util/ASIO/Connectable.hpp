// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <compare>
#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <mcs/util/string.hpp>
#include <type_traits>
#include <variant>

namespace mcs::util::ASIO
{
  template<is_protocol> struct Connectable;
  template<> struct Connectable<asio::ip::tcp>;
  template<> struct Connectable<asio::local::stream_protocol>;

  template<typename Endpoint>
    [[nodiscard]] auto make_connectable (Endpoint const&);

  template<typename C>
    concept is_connectable =
       std::is_same_v<C, Connectable<asio::ip::tcp>>
    || std::is_same_v<C, Connectable<asio::local::stream_protocol>>
    ;
}

namespace mcs::util::ASIO
{
  template<> struct Connectable<asio::ip::tcp>
  {
    explicit Connectable (asio::ip::tcp::endpoint const&);

    struct Address
    {
      util::string address_string;

      auto operator<=> (Address const&) const noexcept = default;
    };
    struct Hostname
    {
      util::string hostname;

      auto operator<=> (Hostname const&) const noexcept= default;
    };

    std::variant<Address, Hostname> address_or_hostname;
    asio::ip::port_type port;

    Connectable (decltype (address_or_hostname), decltype (port));

    auto operator<=> (Connectable const&) const = default;
  };
}

namespace mcs::util::ASIO
{
  template<> struct Connectable<asio::local::stream_protocol>
  {
    explicit Connectable (asio::local::stream_protocol::endpoint const&);

    util::string path;

    explicit Connectable (decltype (path));

    auto operator<=> (Connectable const&) const noexcept = default;
  };
}

namespace mcs::util::ASIO
{
  using AnyConnectable = std::variant
    < Connectable<asio::ip::tcp>
    , Connectable<asio::local::stream_protocol>
    >;

  template<typename Runner, typename... Args>
    auto run (AnyConnectable, Runner&&, Args&&...);
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      (mcs::util::ASIO::Connectable<asio::ip::tcp>)
    ;
  template<>
    MCS_UTIL_FMT_DECLARE
      (mcs::util::ASIO::Connectable<asio::ip::tcp>::Address)
    ;
  template<>
    MCS_UTIL_FMT_DECLARE
      (mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname)
    ;
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (util::ASIO::Connectable<asio::ip::tcp>)
    ;
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (util::ASIO::Connectable<asio::ip::tcp>::Address)
    ;
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (util::ASIO::Connectable<asio::ip::tcp>::Hostname)
    ;
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      (mcs::util::ASIO::Connectable<asio::local::stream_protocol>)
    ;
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (util::ASIO::Connectable<asio::local::stream_protocol>)
    ;
}

namespace std
{
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
    (address_string, mcs::util::ASIO::Connectable<asio::ip::tcp>::Address);
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
    (hostname, mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname);
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
    (address_or_hostname, mcs::util::ASIO::Connectable<asio::ip::tcp>);
  template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
    (path, mcs::util::ASIO::Connectable<asio::local::stream_protocol>);
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::ASIO::Connectable<asio::ip::tcp>
      );
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::ASIO::Connectable<asio::ip::tcp>::Address
      );
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::ASIO::Connectable<asio::ip::tcp>::Hostname
      );
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( util::ASIO::Connectable<asio::local::stream_protocol>
      );
}

#include "detail/Connectable.ipp"
