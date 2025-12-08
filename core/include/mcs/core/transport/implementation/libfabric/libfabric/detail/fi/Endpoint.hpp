// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    struct Endpoint
    {
      struct Deleter
      {
        auto operator() (Endpoint*) const noexcept -> void;
      };

      Endpoint (util::not_null<fid_domain>, util::not_null<fi_info>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fid_ep>;

    private:
      friend struct Deleter;
      util::not_null<fid_ep> _value;
    };
  }

  using Endpoint
    = std::unique_ptr<detail::Endpoint, detail::Endpoint::Deleter>
    ;

  auto make_endpoint
    ( util::not_null<fid_domain>
    , util::not_null<fi_info>
    ) -> Endpoint
    ;
}
