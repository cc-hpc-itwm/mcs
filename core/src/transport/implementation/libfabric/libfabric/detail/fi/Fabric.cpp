// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Fabric.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/invoke_and_throw_on_error.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_errno.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    auto Fabric::Deleter::operator()
      ( Fabric* fabric
      ) const noexcept -> void
    {
      if (auto const err {fi_close (std::addressof (fabric->_value->fid))})
      {
        util::FMT::print_noexcept
          ( stderr
          , "Error: closing fabric: {}"
          , fi_strerror (err)
          );
      }

      std::default_delete<Fabric>{} (fabric);
    }

    Fabric::Fabric (util::not_null<fi_fabric_attr> attr)
      : _value
        { std::invoke
          ( [&]
            {
              fid_fabric* value {nullptr};

              libfabric::detail::invoke_and_throw_on_error
                ( fi_fabric
                , error::LibfabricError::FunctionName {"fi_fabric"}
                , attr.get()
                , std::addressof (value)
                , nullptr
                );

              return value;
            }
          )
        }
    {}

    auto Fabric::get() const noexcept -> util::not_null<fid_fabric>
    {
      return _value;
    }
  }

  auto make_fabric (util::not_null<fi_fabric_attr> attr) -> Fabric
  {
    return Fabric {new detail::Fabric (attr), detail::Fabric::Deleter{}};
  }
}
