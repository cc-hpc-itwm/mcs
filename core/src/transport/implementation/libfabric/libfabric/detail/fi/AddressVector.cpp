// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <functional>
#include <mcs/Error.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/detail/fi/AddressVector.hpp>
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
    auto AddressVector::Deleter::operator()
      ( AddressVector* address_vector
      ) const noexcept -> void
    {
      if ( auto const err
           { fi_close (std::addressof (address_vector->_value->fid))
           }
         )
      {
        util::FMT::print_noexcept
          ( stderr
          , "Error: closing address vector: {}"
          , fi_strerror (err)
          );
      }

      std::default_delete<AddressVector>{} (address_vector);
    }

    AddressVector::AddressVector (util::not_null<fid_domain> domain)
      : _value
        { std::invoke
          ( [&]
            {
              fid_av* value {nullptr};

              libfabric::detail::invoke_and_throw_on_error
                ( fi_av_open
                , error::LibfabricError::FunctionName {"fi_av_open"}
                , domain.get()
                , std::addressof (_attr)
                , std::addressof (value)
                , nullptr
                );

              return value;
            }
          )
        }
    {}

    auto AddressVector::get() const noexcept -> util::not_null<fid_av>
    {
      return _value;
    }

    auto AddressVector::insert_single_address
      ( void const* address
      , fi_addr_t* fi_addr
      , uint64_t flags
      , void* context
      ) -> void
    {
      if ( auto const number_of_inserted_addresses
             { fi_av_insert
                 ( _value.get()
                 , address
                 , 1
                 , fi_addr
                 , flags
                 , context
                 )
             }
         ; number_of_inserted_addresses != 1
         )
      {
        throw mcs::Error
          { fmt::format
              ( "fi_av_insert: {} != 1"
              , number_of_inserted_addresses
              )
          };
      }
    }
  }

  auto make_address_vector
    ( util::not_null<fid_domain> domain
    ) -> AddressVector
  {
    return AddressVector
      { new detail::AddressVector (domain)
      , detail::AddressVector::Deleter{}
      };
  }
}
