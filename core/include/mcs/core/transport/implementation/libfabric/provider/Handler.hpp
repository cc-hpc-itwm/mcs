// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/Error.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/implementation/libfabric/command/Get.hpp>
#include <mcs/core/transport/implementation/libfabric/command/Put.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/Concepts.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::libfabric::provider
{
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    struct Handler
  {
    Handler ( util::not_null<Transporter>
            , util::not_null
                < Storages<util::type::List<StorageImplementations...>>
                >
            );

    auto operator() (command::Get) const -> command::Get::Response;
    auto operator() (command::Put) const -> command::Put::Response;

    struct Error
    {
      struct CouldNotReadAllData : public mcs::Error
      {
        struct Wanted
        {
          constexpr explicit Wanted (std::size_t) noexcept;
          std::size_t value;
        };
        struct Read
        {
          constexpr explicit Read (std::size_t) noexcept;
          std::size_t value;
        };

        constexpr auto wanted() const noexcept -> Wanted;
        constexpr auto read() const noexcept -> Read;

        MCS_ERROR_COPY_MOVE_DEFAULT (CouldNotReadAllData);

      private:
        template
          < transporter::is_provider
          , storage::is_implementation...
          >
          friend struct Handler;

        CouldNotReadAllData (Wanted, Read) noexcept;

        Wanted _wanted;
        Read _read;
      };
    };

  private:
    util::not_null<Transporter> _transporter;
    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages
      ;
  };
}

#include "detail/Handler.ipp"
