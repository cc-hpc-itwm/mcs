// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Domain.hpp>
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
    auto Domain::Deleter::operator()
      ( Domain* domain
      ) const noexcept -> void
    {
      if (auto const err {fi_close (std::addressof (domain->_value->fid))})
      {
        util::FMT::print_noexcept
          ( stderr
          , "Error: closing domain: {}"
          , fi_strerror (err)
          );
      }

      std::default_delete<Domain>{} (domain);
    }

    Domain::Domain
      ( util::not_null<fid_fabric> fabric
      , util::not_null<fi_info> info
      )
        : _value
          { std::invoke
            ( [&]
              {
                fid_domain* value {nullptr};

                libfabric::detail::invoke_and_throw_on_error
                  ( fi_domain
                  , error::LibfabricError::FunctionName {"fi_domain"}
                  , fabric.get()
                  , info.get()
                  , std::addressof (value)
                  , nullptr
                  );

                return value;
              }
            )
          }
    {}

    auto Domain::get() const noexcept -> util::not_null<fid_domain>
    {
      return _value;
    }
  }

  auto make_domain
    ( util::not_null<fid_fabric> fabric
    , util::not_null<fi_info> info
    ) -> Domain
  {
    return Domain
      { new detail::Domain (fabric, info)
      , detail::Domain::Deleter{}
      };
  }
}
