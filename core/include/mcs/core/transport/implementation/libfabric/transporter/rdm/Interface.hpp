// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/config.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/AddressVector.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/CompletionQueue.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Domain.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Endpoint.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Fabric.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Hints.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Info.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/invoke_and_throw_on_error.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_eq.h>
#include <span>
#include <vector>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  struct Interface
  {
  public:
    Interface (libfabric::Interface);

    // Quote from fi_msg(3)
    //
    // An endpoint must be enabled before an application can post send or
    // receive operations to it. For connected endpoints, receive buffers may
    // be posted prior to connect or accept being called on the endpoint.
    // This ensures that buffers are available to receive incoming data
    // immediately after the connection has been established.

    // Receives data.size() many bytes, that is the complete span
    auto recv (std::span<std::byte>) -> std::size_t;

    // Sends data.size() many bytes, that is the complete span
    auto send (std::span<std::byte const>, fi_addr_t) -> void;

    // \todo implement an async recv
    // auto async_recv (std::span<std::byte>) -> future<std::size_t>
    // {
    //   auto promise;
    //   // initiate async recv
    //   return promise.get_future();
    // }

    // \todo implement an async send
    // auto async_send (std::span<std::byte const>, fi_addr_t) -> void
    // {
    //   // initiate async send
    // }

    [[nodiscard]] auto name() const -> libfabric::Name;

  protected:
    libfabric::detail::fi::Hints _hints;
    libfabric::detail::fi::Info _info;
    libfabric::detail::fi::Fabric _fabric
      { libfabric::detail::fi::make_fabric (_info->get()->fabric_attr)
      };
    libfabric::detail::fi::Domain _domain
      { libfabric::detail::fi::make_domain (_fabric->get(), _info->get())
      };
    libfabric::detail::fi::CompletionQueue _cq
      { libfabric::detail::fi::make_completion_queue (_domain->get())
      };
    libfabric::detail::fi::AddressVector _av
      { libfabric::detail::fi::make_address_vector (_domain->get())
      };
    libfabric::detail::fi::Endpoint _ep
      { libfabric::detail::fi::make_endpoint (_domain->get(), _info->get())
      };

    libfabric::Name _name
      { std::invoke
        ( [&]
          {
            auto address_size_wanted {size_t {0}};
            auto const error
              { fi_getname
                  ( std::addressof (_ep->get()->fid)
                  , nullptr
                  , std::addressof (address_size_wanted)
                  )
              };

            if (  std::cmp_not_equal (error, -FI_ETOOSMALL)
               || std::cmp_less_equal (address_size_wanted, 0)
               )
            {
              throw error::LibfabricError
                { error::LibfabricError::FunctionName {"fi_getname"}
                , error::LibfabricError::ErrorName {"invalid size"}
                };
            }

            auto address_size {address_size_wanted};
            auto address_buffer {std::vector<std::byte>
              {address_size_wanted}};

            libfabric::detail::invoke_and_throw_on_error
              ( fi_getname
              , error::LibfabricError::FunctionName {"fi_getname"}
              , std::addressof (_ep->get()->fid)
              , address_buffer.data()
              , std::addressof (address_size)
              );

            if (address_size != address_size_wanted)
            {
              throw error::LibfabricError
                { error::LibfabricError::FunctionName {"fi_getname"}
                , error::LibfabricError::ErrorName {"inconsistent size"}
                };
            }

            return address_buffer;
          }
        )
      };

    auto cq_read() -> fi_cq_err_entry;

    template<typename Description, typename Fun, typename Data>
      auto do_complete_transfer
      ( Description
      , Fun
      , Data
      , fi_addr_t
      );
  };
}

#include "detail/Interface.ipp"
