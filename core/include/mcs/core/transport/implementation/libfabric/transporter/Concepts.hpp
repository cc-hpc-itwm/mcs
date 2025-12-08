// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstddef>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <span>

namespace mcs::core::transport::implementation::libfabric::transporter
{
  namespace detail
  {
    template<typename T>
      concept is_constructible_with_interface
        = requires (libfabric::Interface interface)
      {
        { T {interface}
        };
      };

    template<typename T>
      concept is_constructible_with_interface_and_name
        = requires ( libfabric::Interface interface
                   , libfabric::Name name
                   )
      {
        { T {interface, name}
        };
      };

    template<typename T>
      concept has_recv_method
        = requires (T t, std::span<std::byte> data)
      {
        { t.recv (data) } -> std::same_as<std::size_t>;
      };

    template<typename T>
      concept has_send_method
        = requires (T t, std::span<std::byte const> data)
      {
        { t.send (data) } -> std::same_as<void>;
      };

    template<typename T>
      concept has_send_method_with_name
        = requires ( T t
                   , std::span<std::byte const> data
                   , libfabric::Name name
                   )
      {
        { t.send (data, name) } -> std::same_as<void>;
      };

    template<typename T>
      concept has_name_method
        = requires (T const t)
      {
        { t.name() } -> std::same_as<libfabric::Name>;
      };
  }

  template<typename T>
    concept is_provider
      =  detail::is_constructible_with_interface<T>
      && detail::has_recv_method<T>
      && detail::has_send_method_with_name<T>
      && detail::has_name_method<T>
      ;

  template<typename T>
    concept is_client
      =  detail::is_constructible_with_interface_and_name<T>
      && detail::has_recv_method<T>
      && detail::has_send_method<T>
      && detail::has_name_method<T>
      ;
}
