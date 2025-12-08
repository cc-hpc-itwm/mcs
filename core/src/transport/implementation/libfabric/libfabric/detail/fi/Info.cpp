// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdint>
#include <functional>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/Info.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/invoke_and_throw_on_error.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <rdma/fabric.h>

namespace mcs::core::transport::implementation::libfabric::libfabric::detail::fi
{
  namespace detail
  {
    auto Info::Deleter::operator() (Info* info) const noexcept -> void
    {
      fi_freeinfo (info->_value.get());

      std::default_delete<Info>{} (info);
    }

    Info::Info
      ( util::not_null<fi_info> hints
      )
        : _value
          { std::invoke
            ( [&]
              {
                fi_info* value {nullptr};

                libfabric::detail::invoke_and_throw_on_error
                  ( fi_getinfo
                  , error::LibfabricError::FunctionName {"fi_getinfo"}
                  , FI_VERSION (1, 11)
                  , nullptr
                  , nullptr
                  , std::uint64_t {0}
                  , hints.get()
                  , std::addressof (value)
                  );

                return value;
            }
          )
      }
    {}

    auto Info::get() const noexcept -> util::not_null<fi_info>
    {
      return _value;
    }
  }

  auto make_info
    ( util::not_null<fi_info> hints
    ) -> Info
  {
    return Info { new detail::Info {hints}
                , detail::Info::Deleter{}
                };
  }
}
