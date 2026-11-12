// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <future>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>

namespace mcs::core::transport::client
{
  namespace detail
  {
    template<typename Client>
      concept has_memory_get = requires
        ( Client& client
        , Address destination
        , Address source
        , memory::Size size
        )
      {
        { client.memory_get
           ( destination
           , source
           , size
           )
        } -> std::convertible_to<std::future<memory::Size>>;
      };

    template<typename Client>
      concept has_memory_put = requires
        ( Client& client
        , Address destination
        , Address source
        , memory::Size size
        )
      {
        { client.memory_put
           ( destination
           , source
           , size
           )
        } -> std::convertible_to<std::future<memory::Size>>;
      };
  }

  template<typename Client>
    concept is_stateful_implementation =
       detail::has_memory_get<Client>
    && detail::has_memory_put<Client>
    ;

  template<typename Client>
    concept is_stateless_implementation =
       detail::has_memory_get<Client const>
    && detail::has_memory_put<Client const>
    ;
}
