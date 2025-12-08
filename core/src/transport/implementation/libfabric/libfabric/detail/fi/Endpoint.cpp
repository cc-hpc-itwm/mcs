// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Endpoint.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/invoke_and_throw_on_error.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_errno.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    auto Endpoint::Deleter::operator()
      ( Endpoint* endpoint
      ) const noexcept -> void
    {
      if (auto const err {fi_close (std::addressof (endpoint->_value->fid))})
      {
        util::FMT::print_noexcept
          ( stderr
          , "Error: closing endpoint: {}"
          , fi_strerror (err)
          );
      }

      std::default_delete<Endpoint>{} (endpoint);
    }

    Endpoint::Endpoint
      ( util::not_null<fid_domain> domain
      , util::not_null<fi_info> info
      )
        : _value
          { std::invoke
            ( [&]
              {
                fid_ep* value {nullptr};

                libfabric::detail::invoke_and_throw_on_error
                  ( fi_endpoint
                  , error::LibfabricError::FunctionName {"fi_endpoint"}
                  , domain.get()
                  , info.get()
                  , std::addressof (value)
                  , nullptr
                  );

                return value;
              }
            )
          }
    {}

    auto Endpoint::get() const noexcept -> util::not_null<fid_ep>
    {
      return _value;
    }
  }

  auto make_endpoint
    ( util::not_null<fid_domain> domain
    , util::not_null<fi_info> info
    ) -> Endpoint
  {
    return Endpoint
      { new detail::Endpoint (domain, info)
      , detail::Endpoint::Deleter{}
      };
  }
}
