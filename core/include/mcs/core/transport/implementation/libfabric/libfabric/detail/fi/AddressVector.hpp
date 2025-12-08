// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    struct AddressVector
    {
      struct Deleter
      {
        auto operator() (AddressVector*) const noexcept -> void;
      };

      AddressVector (util::not_null<fid_domain>);

      [[nodiscard]] auto get() const noexcept -> util::not_null<fid_av>;

      auto insert_single_address
        ( void const* address
        , fi_addr_t*
        , uint64_t flags
        , void* context
        ) -> void
        ;

    private:
      friend struct Deleter;
      fi_av_attr _attr{};
      util::not_null<fid_av> _value;
    };
  }

  using AddressVector
    = std::unique_ptr<detail::AddressVector, detail::AddressVector::Deleter>
    ;

  auto make_address_vector (util::not_null<fid_domain>) -> AddressVector;
}
