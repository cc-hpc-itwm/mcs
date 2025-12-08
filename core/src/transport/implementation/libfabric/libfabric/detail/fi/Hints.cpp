// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/base.h>
#include <fmt/format.h>
#include <mcs/Error.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Hints.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/syscall/strdup.hpp>
#include <memory>
#include <rdma/fabric.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    auto Hints::Deleter::operator()
      ( Hints* hints
      ) const noexcept -> void
    {
      fi_freeinfo (hints->_value.get());

      std::default_delete<Hints>{} (hints);
    }

    Hints::Hints (libfabric::Interface interface)
    try
        : _value {fi_allocinfo()}
    {
      _value->ep_attr->type = FI_EP_RDM;
      _value->caps = FI_MSG;
      _value->fabric_attr->prov_name = util::syscall::strdup
        ( interface.provider.value().data()
        );
      _value->domain_attr->name = util::syscall::strdup
        ( interface.domain.value().data()
        );
      _value->addr_format = FI_SOCKADDR_IN;
      _value->tx_attr->op_flags = FI_DELIVERY_COMPLETE;
    }
    catch (...)
    {
      std::throw_with_nested
        ( mcs::Error
          { fmt::format ("Hints::Hints ({})", interface.provider.value())
          }
        );
    }

    auto Hints::get() const noexcept -> util::not_null<fi_info>
    {
      return _value;
    }
  }

  auto make_hints (libfabric::Interface interface) -> Hints
  {
    return Hints {new detail::Hints {interface}, detail::Hints::Deleter{}};
  }
}
