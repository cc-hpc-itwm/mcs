// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/CompletionQueue.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/invoke_and_throw_on_error.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_errno.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    auto CompletionQueue::Deleter::operator()
      ( CompletionQueue* completion_queue
      ) const noexcept -> void
    {
      if ( auto const err
           { fi_close (std::addressof (completion_queue->_value->fid))
           }
         )
      {
        util::FMT::print_noexcept
          ( stderr
          , "Error: closing completion queue: {}"
          , fi_strerror (err)
          );
      }

      std::default_delete<CompletionQueue>{} (completion_queue);
    }

    CompletionQueue::CompletionQueue (util::not_null<fid_domain> domain)
      : _value
        { std::invoke
          ( [&]
            {
              fid_cq* value {nullptr};

              libfabric::detail::invoke_and_throw_on_error
                ( fi_cq_open
                , error::LibfabricError::FunctionName {"fi_cp_open"}
                , domain.get()
                , std::addressof (_attr)
                , std::addressof (value)
                , nullptr
                );

              return value;
            }
          )
        }
    {}

    auto CompletionQueue::get() const noexcept -> util::not_null<fid_cq>
    {
      return _value;
    }
  }

  auto make_completion_queue
    ( util::not_null<fid_domain> domain
    ) -> CompletionQueue
  {
    return CompletionQueue
      { new detail::CompletionQueue (domain)
      , detail::CompletionQueue::Deleter{}
      };
  }
}
