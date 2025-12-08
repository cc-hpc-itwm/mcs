// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_eq.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    struct CompletionQueue
    {
      struct Deleter
      {
        auto operator() (CompletionQueue*) const noexcept -> void;
      };

      CompletionQueue (util::not_null<fid_domain>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fid_cq>;

    private:
      friend struct Deleter;
      fi_cq_attr _attr
        { std::invoke
          ( []() noexcept
            {
              fi_cq_attr attr{};
              attr.format = FI_CQ_FORMAT_MSG;
              return attr;
            }
          )
        };
      util::not_null<fid_cq> _value;
    };
  }

  using CompletionQueue
    = std::unique_ptr< detail::CompletionQueue
                     , detail::CompletionQueue::Deleter
                     >
    ;

  auto make_completion_queue (util::not_null<fid_domain>) -> CompletionQueue;
}
