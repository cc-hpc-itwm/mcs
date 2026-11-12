// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/awaitable.hpp>
#include <asio/strand.hpp>
#include <cstddef>
#include <deque>
#include <exception>
#include <future>
#include <map>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/detail/CallID.hpp>
#include <mcs/rpc/detail/Completion.hpp>
#include <mcs/rpc/detail/ReceivedFrame.hpp>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace mcs::rpc::detail::client
{
  template<is_protocol Protocol, typename AdmissionTag>
    struct Connection
      : std::enable_shared_from_this<Connection<Protocol, AdmissionTag>>
  {
    explicit Connection (std::shared_ptr<typename Protocol::socket>);

    template<typename MakeSend>
      auto submit (MakeSend, detail::Completion) -> void;

    [[nodiscard]] auto shutdown() -> std::future<void>;

  private:
    struct PendingCall
    {
      detail::Completion complete;
      bool sent {false};
      std::optional<ReceivedFrame<CallID>> response{};
    };

    struct Send
    {
      CallID call_id;
      asio::awaitable<void> operation;
      PendingCall call;
    };

    auto start_sending() -> void;
    auto start_receiving() -> void;

    static auto send_requests
      ( std::shared_ptr<Connection> self
      ) -> asio::awaitable<void>
      ;
    static auto receive_responses
      ( std::shared_ptr<Connection> self
      ) -> asio::awaitable<void>
      ;

    [[nodiscard]] auto front_send() -> Send*;
    auto take_send (Send&) -> Send;
    auto call (CallID) -> PendingCall&;
    auto take_call (CallID) -> PendingCall;
    auto deliver (ReceivedFrame<CallID>) -> void;

    auto close (std::exception_ptr) -> void;

    auto finish_shutdown() -> void;

    std::shared_ptr<typename Protocol::socket> _socket;
    asio::strand<asio::any_io_executor> _executor;
    std::conditional_t
      < std::same_as<AdmissionTag, access_policy::Exclusive>
      , std::optional<Send>
      , std::deque<Send>
      > _sends;
    std::conditional_t
      < std::same_as<AdmissionTag, access_policy::Concurrent>
      , std::map<CallID, PendingCall>
      , std::optional<std::pair<CallID, PendingCall>>
      > _completions;
    CallID _next_call_id;
    std::size_t _expected_responses {0};
    bool _sending {false};
    bool _receiving {false};
    std::exception_ptr _terminal_error;
    std::exception_ptr _socket_close_error;
    std::vector<std::promise<void>> _shutdown_waiters;
  };
}

#include "detail/Connection.ipp"
