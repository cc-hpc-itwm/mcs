// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/declare.hpp>
#include <memory>

namespace mcs::core::transport::implementation::ASIO::command
{
  struct Get
  {
    using Response = core::memory::Size;

    core::transport::Address source;
    core::memory::Size size;
    struct Destination
    {
      virtual auto data() const -> std::span<std::byte> = 0;
      virtual ~Destination() = default;
    };
    std::unique_ptr<Destination> destination;

    template<typename Socket>
      auto stream (Socket&) const -> void;

    struct Error
    {
      struct CouldNotReadAllData : public mcs::Error
      {
        struct Wanted
        {
          constexpr explicit Wanted (core::memory::Size) noexcept;
          core::memory::Size value;
        };
        struct Read
        {
          constexpr explicit Read (core::memory::Size) noexcept;
          core::memory::Size value;
        };

        constexpr auto wanted() const noexcept -> Wanted;
        constexpr auto read() const noexcept -> Read;

        MCS_ERROR_COPY_MOVE_DEFAULT (CouldNotReadAllData);

      private:
        friend struct Get;

        CouldNotReadAllData (Wanted, Read) noexcept;

        Wanted _wanted;
        Read _read;
      };
    };
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::transport::implementation::ASIO::command::Get)
    ;
}

#include "detail/Get.ipp"
