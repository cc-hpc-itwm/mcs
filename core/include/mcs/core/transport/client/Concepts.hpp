// Copyright (C) 2025 Fraunhofer ITWM
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
        ( Client const& client
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
        ( Client const& client
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
    concept is_implementation =
       detail::has_memory_get<Client>
    && detail::has_memory_put<Client>
    ;
}
