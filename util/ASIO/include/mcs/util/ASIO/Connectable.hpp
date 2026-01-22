// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <compare>
#include <fmt/base.h>
#include <functional>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/STD/variant.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/string.hpp>
#include <type_traits>
#include <variant>

namespace mcs::util::ASIO
{
  template<is_protocol> struct Connectable;
  template<>
    struct Connectable<asio::ip::tcp>;
  template<>
    struct Connectable<asio::local::stream_protocol>;

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
  template<>
    struct Connectable<asio::ip::tcp>
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
  template<>
    struct Connectable<asio::local::stream_protocol>
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
    struct formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::ASIO::Connectable<asio::ip::tcp> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
  template<>
    struct formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Address>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Address const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
  template<>
    struct formatter<mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
}

namespace mcs::util::read
{
  template<>
    struct Read<util::ASIO::Connectable<asio::ip::tcp>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> util::ASIO::Connectable<asio::ip::tcp>
        ;
  };
  template<>
    struct Read<util::ASIO::Connectable<asio::ip::tcp>::Address>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> util::ASIO::Connectable<asio::ip::tcp>::Address
        ;
  };
  template<>
    struct Read<util::ASIO::Connectable<asio::ip::tcp>::Hostname>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> util::ASIO::Connectable<asio::ip::tcp>::Hostname
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::util::ASIO::Connectable<asio::local::stream_protocol>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::ASIO::Connectable<asio::local::stream_protocol> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
}

namespace mcs::util::read
{
  template<>
    struct Read<util::ASIO::Connectable<asio::local::stream_protocol>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> util::ASIO::Connectable<asio::local::stream_protocol>
        ;
  };
}

namespace std
{
  template<>
    struct hash<mcs::util::ASIO::Connectable<asio::ip::tcp>::Address>
  {
    using Address = mcs::util::ASIO::Connectable<asio::ip::tcp>::Address;

    auto operator() (Address const&) const noexcept -> size_t;

  private:
    hash<decltype (Address::address_string)> _hash;
  };

  template<>
    struct hash<mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname>
  {
    using Hostname = mcs::util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    auto operator() (Hostname const&) const noexcept -> size_t;

  private:
    hash<decltype (Hostname::hostname)> _hash;
  };

  template<>
    struct hash<mcs::util::ASIO::Connectable<asio::ip::tcp>>
  {
    using Connectable = mcs::util::ASIO::Connectable<asio::ip::tcp>;

    auto operator() (Connectable const&) const noexcept -> size_t;

  private:
    hash<decltype (Connectable::address_or_hostname)> _hash;
  };

  template<>
    struct hash<mcs::util::ASIO::Connectable<asio::local::stream_protocol>>
  {
    using Connectable
      = mcs::util::ASIO::Connectable<asio::local::stream_protocol>
      ;

    auto operator() (Connectable const&) const noexcept -> size_t;

  private:
    hash<decltype (Connectable::path)> _hash;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<util::ASIO::Connectable<asio::ip::tcp>>
  {
    using Type = util::ASIO::Connectable<asio::ip::tcp>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
  template<>
    struct Implementation<util::ASIO::Connectable<asio::ip::tcp>::Address>
  {
    using Type = util::ASIO::Connectable<asio::ip::tcp>::Address;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
  template<>
    struct Implementation<util::ASIO::Connectable<asio::ip::tcp>::Hostname>
  {
    using Type = util::ASIO::Connectable<asio::ip::tcp>::Hostname;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
  template<>
    struct Implementation<util::ASIO::Connectable<asio::local::stream_protocol>>
  {
    using Type = util::ASIO::Connectable<asio::local::stream_protocol>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/Connectable.ipp"
