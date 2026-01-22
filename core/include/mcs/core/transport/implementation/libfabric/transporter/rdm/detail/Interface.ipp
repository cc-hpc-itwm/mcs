// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/transport/implementation/libfabric/error/LibfabricError.hpp>
#include <mcs/util/cast.hpp>
#include <rdma/fi_errno.h>
#include <tuple>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  template<typename Description, typename Fun, typename Data>
    auto Interface::do_complete_transfer
    ( Description description
    , Fun fun
    , Data data
    , fi_addr_t addr
    )
  {
  TRY_TRANSFER:
    auto const error_or_try_again
      { std::invoke ( fun
                    , _ep->get().get()
                    , data.data()
                    , data.size()
                    , nullptr
                    , addr
                    , nullptr
                    )
      };

    if (std::cmp_not_equal (error_or_try_again, 0))
    {
      if (std::cmp_equal (error_or_try_again, -FI_EAGAIN))
      {
        // Quote from fi_msg(3)
        //
        // -FI_EAGAIN
        //
        // Indicates that the underlying provider currently lacks the
        // resources needed to initiate the requested operation. The reasons
        // for a provider returning FI_EAGAIN are varied. However, common
        // reasons include insufficient internal buffering or full processing
        // queues.
        //
        // Insufficient internal buffering is often associated with operations
        // that use FI_INJECT. In such cases, additional buffering may become
        // available as posted operations complete.
        //
        // Full processing queues may be a temporary state related to local
        // processing (for example, a large message is being transferred), or
        // may be the result of flow control. In the latter case, the queues
        // may remain blocked until additional resources are made available at
        // the remote side of the transfer.
        //
        // In all cases, the operation may be retried after additional
        // resources become available. When using FI_PROGRESS_MANUAL, the
        // application must check for transmit and receive completions after
        // receiving FI_EAGAIN as a return value, independent of the operation
        // which failed. This is also strongly recommended when using
        // FI_PROGRESS_AUTO, as acknowledgements or flow control messages may
        // need to be processed in order to resume execution.

        // \note Return value is ignored here as we are not actually reading
        // anything (i.e. last argument is 0). This is used to drive progress
        // as mentioned in the documentation. We expect the return value to be
        // 0 (i.e. number of completions retrieved from the completion queue),
        // but it can also return a negative fabric errno on error.
        std::ignore = fi_cq_read (_cq->get().get(), nullptr, 0);

        goto TRY_TRANSFER;
      }

      throw error::LibfabricError
        { error::LibfabricError::FunctionName {description}
        , error::LibfabricError::ErrorName
          { fi_strerror (util::cast<int> (error_or_try_again))
          }
        };
    }
  }
}
