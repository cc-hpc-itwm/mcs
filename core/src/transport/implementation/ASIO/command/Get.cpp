// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/transport/implementation/ASIO/command/Get.hpp>
#include <mcs/serialization/define.hpp>

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
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , get
    , core::transport::implementation::ASIO::command::Get
    )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, get, source);
    MCS_SERIALIZATION_SAVE_FIELD (oa, get, size);

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , core::transport::implementation::ASIO::command::Get
    )
  {
    namespace ASIO = core::transport::implementation::ASIO;
    using Get = ASIO::command::Get;

    MCS_SERIALIZATION_LOAD_FIELD (ia, source, Get);
    MCS_SERIALIZATION_LOAD_FIELD (ia, size, Get);

    return Get {source, size, nullptr};
  }
}
