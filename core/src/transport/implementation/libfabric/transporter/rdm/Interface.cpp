// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cassert>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/Error.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Interface.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_eq.h>
#include <rdma/fi_errno.h>
#include <span>
#include <tuple>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  Interface::Interface
    ( libfabric::Interface interface
    )
      : _hints {libfabric::detail::fi::make_hints (interface)}
      , _info {libfabric::detail::fi::make_info (_hints->get())}
  {
    libfabric::detail::invoke_and_throw_on_error
      ( fi_ep_bind
      , error::LibfabricError::FunctionName {"fi_ep_bind"}
      , _ep->get().get()
      , std::addressof (_cq->get()->fid)
      , FI_SEND | FI_RECV
      );
    libfabric::detail::invoke_and_throw_on_error
      ( fi_ep_bind
      , error::LibfabricError::FunctionName {"fi_ep_bind"}
      , _ep->get().get()
      , std::addressof (_av->get()->fid)
      , 0
      );
    libfabric::detail::invoke_and_throw_on_error
      ( fi_enable
      , error::LibfabricError::FunctionName {"fi_enable"}
      , _ep->get().get()
      );
  }

  auto Interface::recv (std::span<std::byte> data) -> std::size_t
  {
    do_complete_transfer ("fi_recv", fi_recv, data, FI_ADDR_UNSPEC);
    auto const comp {cq_read()};
    auto const bytes_read {comp.len};
    return bytes_read;
  }

  auto Interface::send
    ( std::span<std::byte const> data
    , fi_addr_t addr
    ) -> void
  {
    do_complete_transfer ("fi_send", fi_send, data, addr);
    std::ignore = cq_read();
  }

  auto Interface::name() const -> libfabric::Name
  {
    return _name;
  }

  auto Interface::cq_read() -> fi_cq_err_entry
  {
    auto comp {fi_cq_err_entry{}};

  TRY_CQ_READ:
    auto const number_of_completions_or_error_or_try_again
      { fi_cq_read (_cq->get().get(), std::addressof (comp), 1)
      };

    if (std::cmp_not_equal (number_of_completions_or_error_or_try_again, 1))
    {
      if ( std::cmp_equal
           ( number_of_completions_or_error_or_try_again
           , -FI_EAGAIN
           )
         )
      {
        goto TRY_CQ_READ;
      }

      throw error::LibfabricError
        { error::LibfabricError::FunctionName {"fi_cq_read"}
        , error::LibfabricError::ErrorName
          { fi_strerror
              ( util::cast<int> (number_of_completions_or_error_or_try_again)
              )
          }
        };
    }

    return comp;
  }
}
