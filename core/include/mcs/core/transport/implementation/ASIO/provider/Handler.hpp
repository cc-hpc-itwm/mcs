// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/Error.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/implementation/ASIO/command/Get.hpp>
#include <mcs/core/transport/implementation/ASIO/command/Put.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::core::transport::implementation::ASIO::provider
{
  template<storage::is_implementation... StorageImplementations>
    struct Handler
  {
    Handler
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
      );

    template<typename Socket>
      auto operator()
        ( command::Get
        , Socket&
        ) const -> command::Get::Response
      ;
    template<typename Socket>
      auto operator()
        ( command::Put
        , Socket&
        ) const -> command::Put::Response
      ;

    struct Error
    {
      struct CouldNotWriteAllData : public mcs::Error
      {
        struct Wanted
        {
          constexpr explicit Wanted (std::size_t) noexcept;
          std::size_t value;
        };
        struct Written
        {
          constexpr explicit Written (std::size_t) noexcept;
          std::size_t value;
        };

        [[nodiscard]] constexpr auto wanted() const noexcept -> Wanted;
        [[nodiscard]] constexpr auto written() const noexcept -> Written;

        MCS_ERROR_COPY_MOVE_DEFAULT (CouldNotWriteAllData);

      private:
        template<storage::is_implementation...> friend struct Handler;

        CouldNotWriteAllData (Wanted, Written) noexcept;

        Wanted _wanted;
        Written _written;
      };

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
        template<storage::is_implementation...> friend struct Handler;

        CouldNotReadAllData (Wanted, Read) noexcept;

        Wanted _wanted;
        Read _read;
      };
    };

  private:
    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages;
  };
}

#include "detail/Handler.ipp"
