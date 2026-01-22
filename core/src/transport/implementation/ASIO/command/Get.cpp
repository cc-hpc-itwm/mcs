// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/transport/implementation/ASIO/command/Get.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::core::transport::implementation::ASIO::command
{
  Get::Error::CouldNotReadAllData::CouldNotReadAllData
    ( Wanted wanted
    , Read read
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "mcs::core::transport::implementation::ASIO::command::Get::CouldNotReadAllData:"
            " wanted: {}, written: {}"
          , wanted.value
          , read.value
          )
        }
      , _wanted {wanted}
      , _read {read}
  {}
  Get::Error::CouldNotReadAllData::~CouldNotReadAllData() = default;
}

namespace mcs::serialization
{
  auto Implementation<core::transport::implementation::ASIO::command::Get>::output
    ( OArchive& oa
    , core::transport::implementation::ASIO::command::Get const& get
    ) -> OArchive&
  {
    save (oa, get.source);
    save (oa, get.size);

    return oa;
  }

  auto Implementation<core::transport::implementation::ASIO::command::Get>::input
    ( IArchive& ia
    ) -> core::transport::implementation::ASIO::command::Get
  {
    namespace ASIO = core::transport::implementation::ASIO;
    using Get = ASIO::command::Get;

    auto source {load<decltype (Get::source)> (ia)};
    auto size {load<decltype (Get::size)> (ia)};

    return Get {source, size, nullptr};
  }
}
