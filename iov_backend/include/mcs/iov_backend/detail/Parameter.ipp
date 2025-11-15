// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::iov_backend::Parameter)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (parameter, ctx, mcs::iov_backend::Parameter)
  {
    return fmt::format_to
      ( ctx.out()
      , "mcs::iov_backend::Parameter\n"
        "[ provider = {}\n"
        ", NumberOfThreads\n"
        "  [ transport_clients = {}\n"
        "  ]\n"
        ", IndirectCommunication\n"
        "  [ number_of_buffers = {}\n"
        "  , maximum_transfer_size = {}\n"
        "  , acquire_buffer_timeout_in_milliseconds = {}\n"
        "  , maximum_number_of_parallel_streams = {}\n"
        "  ]\n"
        ", DirectCommunication\n"
        "  [ maximum_transfer_size = {}\n"
        "  , maximum_number_of_parallel_streams = {}\n"
        "  ]\n"
        "]\n"
      , parameter._provider
      , parameter._number_of_threads._transport_clients
      , parameter._indirect_communication._number_of_buffers
      , parameter._indirect_communication._maximum_transfer_size
      , parameter._indirect_communication._acquire_buffer_timeout_in_milliseconds
      , parameter._indirect_communication._maximum_number_of_parallel_streams
      , parameter._direct_communication._maximum_transfer_size
      , parameter._direct_communication._maximum_number_of_parallel_streams
      );
  }
}

#include <mcs/util/read/parse.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/skip_whitespace.hpp>

namespace mcs::util::read
{
  namespace
  {
    template<typename Value, typename Key, typename State>
      auto key_value (Key key, State& state) -> Value
    {
      prefix (state, key);
      prefix (state, "=");
      return parse<Value> (state);
    }
  }

  // \todo use InBrackets, Symbol, Comma
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    ( state
    , iov_backend::Parameter
    )
  {
    prefix (state, "mcs::iov_backend::Parameter");
    prefix (state, "[");
    auto provider {key_value<util::ASIO::AnyConnectable> ("provider", state)};
    prefix (state, ",");
    prefix (state, "NumberOfThreads");
    prefix (state, "[");
    auto number_of_threads_transport_clients
      { key_value<rpc::ScopedRunningIOContext::NumberOfThreads>
         ( "transport_clients"
         , state
         )
      };
    prefix (state, "]");
    prefix (state, ",");
    prefix (state, "IndirectCommunication");
    prefix (state, "[");
    auto indirect_communication_number_of_buffers
      {key_value<std::size_t> ("number_of_buffers", state)};
    prefix (state, ",");
    auto indirect_communication_maximum_transfer_size
      {key_value<core::memory::Size> ("maximum_transfer_size", state)};
    prefix (state, ",");
    auto indirect_communication_acquire_buffer_timeout_in_milliseconds
      {key_value<std::size_t> ("acquire_buffer_timeout_in_milliseconds", state)};
    prefix (state, ",");
    auto indirect_communication_maximum_number_of_parallel_streams
      {key_value<std::size_t> ("maximum_number_of_parallel_streams", state)};
    prefix (state, "]");
    prefix (state, ",");
    prefix (state, "DirectCommunication");
    prefix (state, "[");
    auto direct_communication_maximum_transfer_size
      {key_value<core::memory::Size> ("maximum_transfer_size", state)};
    prefix (state, ",");
    auto direct_communication_maximum_number_of_parallel_streams
      {key_value<std::size_t> ("maximum_number_of_parallel_streams", state)};
    prefix (state, "]");
    prefix (state, "]");
    skip_whitespace (state);

    return iov_backend::Parameter
      { ._provider = provider
      , ._number_of_threads
          = iov_backend::Parameter::NumberOfThreads
            { ._transport_clients
                = number_of_threads_transport_clients
            }
      , ._indirect_communication
          = iov_backend::Parameter::IndirectCommunication
            { ._number_of_buffers = indirect_communication_number_of_buffers
            , ._maximum_transfer_size
                = indirect_communication_maximum_transfer_size
            , ._acquire_buffer_timeout_in_milliseconds
                = indirect_communication_acquire_buffer_timeout_in_milliseconds
            , ._maximum_number_of_parallel_streams
                = indirect_communication_maximum_number_of_parallel_streams
            }
      , ._direct_communication
          = iov_backend::Parameter::DirectCommunication
            { ._maximum_transfer_size
                = direct_communication_maximum_transfer_size
            , ._maximum_number_of_parallel_streams
                = direct_communication_maximum_number_of_parallel_streams
            }
      };
  }
}
