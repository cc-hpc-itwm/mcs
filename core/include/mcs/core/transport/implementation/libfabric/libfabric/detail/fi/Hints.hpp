// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    struct Hints
    {
      struct Deleter
      {
        auto operator() (Hints*) const noexcept -> void;
      };

      Hints (libfabric::Interface);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fi_info>;

    private:
      friend struct Deleter;
      util::not_null<fi_info> _value;
    };
  }

  using Hints = std::unique_ptr<detail::Hints, detail::Hints::Deleter>;

  auto make_hints (libfabric::Interface) -> Hints;
}
