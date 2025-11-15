// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/storage/Implement_C_API.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>

namespace mcs::core::storage::implement_c_api
{
  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel> channel
    , char const* string
    ) noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "Could not print into channel"
      , [&]
        {
          return channel->append
            ( channel->sink
            , util::cast<::MCS_CORE_STORAGE_BYTE const*> (string)
            , std::strlen (string)
            );
        }
      );
  }

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel> channel
    , std::string data
    ) noexcept -> void
  try
  {
    return channel->append
      ( channel->sink
      , util::cast<::MCS_CORE_STORAGE_BYTE const*> (data.data())
      , data.size()
      );
  }
  catch (...)
  {
    return write_into_channel (channel, "Could not append to channel.\0");
  }

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel> channel
    , std::exception_ptr error
    ) noexcept -> void
  try
  {
    return write_into_channel (channel, fmt::format ("{}", error));
  }
  catch (...)
  {
    return write_into_channel (channel, "Could not make error.\0");
  }

  auto write_into_channel
    ( util::not_null<::mcs_core_storage_channel> channel
    , ::MCS_CORE_STORAGE_SIZE size
    ) noexcept -> void
  try
  {
    return channel->append
      ( channel->sink
      , util::cast<::MCS_CORE_STORAGE_BYTE const*> (std::addressof (size))
      , sizeof (size)
      );
  }
  catch (...)
  {
    return write_into_channel (channel, "Could not write size.\0");
  }
}

namespace mcs::core::storage
{
  Implement_C_API::Error::InstanceMustNotBeNull::InstanceMustNotBeNull()
    : mcs::Error {"Instance must not be null"}
  {}
  Implement_C_API::Error::InstanceMustNotBeNull::~InstanceMustNotBeNull
    (
    ) = default
    ;
}
