// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <mcs/rpc/detail/CommandIndex.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <utility>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_command Command
          , is_command... Commands
          , typename CommandHolder
          >
    auto send_command
      ( std::shared_ptr<typename Protocol::socket> socket
      , CallID call_id
      , CommandHolder command
      ) -> asio::awaitable<void>
  {
    auto constexpr index {command_index<Command, Commands...>()};

    auto oa {serialization::OArchive {call_id, index, command.ref()}};

    co_await asio::async_write (*socket, oa.buffers(), asio::use_awaitable);

    if constexpr (command_is_streaming<Command, typename Protocol::socket>)
    {
      command.ref().stream (*socket);
    }
  }

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command Command
          , is_command... Commands
          , typename CommandHolder
          >
    requires (is_one_of_the_commands<Command, Commands...>)
    auto remote_call
      ( std::shared_ptr<client::Connection<Protocol, typename AccessPolicy::admission>> connection
      , std::shared_ptr<AccessPolicy> access_policy
      , CommandHolder command
      ) -> std::future<typename Command::Response>
  {
    auto promise {std::promise<typename Command::Response>{}};
    auto future {promise.get_future()};

    auto completion {Completion {std::move (promise), access_policy}};

    try
    {
      connection->submit
        ( [command = std::move (command)]
            ( std::shared_ptr<typename Protocol::socket> socket
            , CallID call_id
            ) mutable
          {
            return send_command<Protocol, Command, Commands...>
              ( std::move (socket)
              , call_id
              , std::move (command)
              );
          }
        , std::move (completion)
        );
    }
    catch (...)
    {
      if constexpr (has_error_execution<AccessPolicy>)
      {
        access_policy->error_execution();
      }

      throw;
    }

    return future;
  }
}
