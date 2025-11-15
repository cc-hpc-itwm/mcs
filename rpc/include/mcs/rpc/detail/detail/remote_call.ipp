// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/write.hpp>
#include <exception>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/rpc/detail/receive_buffer_with_header.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <tuple>
#include <utility>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command Command
          , is_command... Commands
          , typename CommandHolder
          >
    requires (is_one_of_the_commands<Command, Commands...>)
    auto remote_call
      ( ClientState<Protocol, AccessPolicy, Commands...> client
      , CommandHolder command
      ) -> std::future<typename Command::Response>
  {
    auto promise {std::promise<typename Command::Response>{}};
    auto future {promise.get_future()};
    auto const call_id
      {client.access_policy->start_call (Completion {std::move (promise)})};
    auto constexpr index {command_index<Command, Commands...>()};

    auto oa {serialization::OArchive { call_id
                                     , index
                                     , command.ref()
                                     }
            };

    asio::write (*client.socket, oa.buffers());

    if constexpr (command_is_streaming<Command, typename Protocol::socket>)
    {
      command.ref().stream (*client.socket);
    }

    if constexpr (needs_sent_notification<AccessPolicy>)
    {
      client.access_policy->sent();
    }

    auto completion
      { [access_policy = client.access_policy]
          ( std::exception_ptr rpc_error
          , std::tuple<CallID, Buffer> response
          )
        {
          if (rpc_error)
          {
            access_policy->error (rpc_error);
          }
          else
          {
            // \note completed_call_id might be different from call_id
            auto& [completed_call_id, buffer] {response};

            access_policy->completion (completed_call_id) (std::move (buffer));
          }
        }
      };

    if constexpr (needs_read_lock<AccessPolicy>)
    {
      asio::co_spawn
        ( client.socket->get_executor()
        , receive_buffer_with_header<Protocol, CallID>
            (client.socket, client.access_policy->read_lock())
        , completion
        );
    }
    else
    {
      asio::co_spawn
        ( client.socket->get_executor()
        , receive_buffer_with_header<Protocol, CallID> (client.socket)
        , completion
        );
    }

    return future;
  }
}
