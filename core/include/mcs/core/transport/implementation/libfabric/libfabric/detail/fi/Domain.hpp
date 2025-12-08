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
    struct Domain
    {
      struct Deleter
      {
        auto operator() (Domain*) const noexcept -> void;
      };

      Domain (util::not_null<fid_fabric>, util::not_null<fi_info>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fid_domain>;

    private:
      friend struct Deleter;
      util::not_null<fid_domain> _value;
    };
  }

  using Domain = std::unique_ptr<detail::Domain, detail::Domain::Deleter>;

  auto make_domain
    ( util::not_null<fid_fabric>
    , util::not_null<fi_info>
    ) -> Domain
    ;
}
