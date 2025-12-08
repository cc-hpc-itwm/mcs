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
    struct Fabric
    {
      struct Deleter
      {
        auto operator() (Fabric*) const noexcept -> void;
      };

      Fabric (util::not_null<fi_fabric_attr>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fid_fabric>;

    private:
      friend struct Deleter;
      util::not_null<fid_fabric> _value;
    };
  }

  using Fabric = std::unique_ptr<detail::Fabric, detail::Fabric::Deleter>;

  auto make_fabric (util::not_null<fi_fabric_attr>) -> Fabric;
}
