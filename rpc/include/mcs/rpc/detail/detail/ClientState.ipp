// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/detail/connected_socket.hpp>
#include <utility>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
      ClientState<Protocol, AccessPolicy, Commands...>::ClientState
        ( std::shared_ptr<typename Protocol::socket> socket_
        , std::shared_ptr<AccessPolicy> access_policy_
        ) noexcept
          : socket {std::move (socket_)}
          , access_policy {access_policy_}
  {}

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<typename Executor>
      ClientState<Protocol, AccessPolicy, Commands...>::ClientState
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        , std::shared_ptr<AccessPolicy> access_policy_
        )
          : ClientState<Protocol, AccessPolicy, Commands...>
            { detail::connected_socket<Protocol, Commands...>
                (executor, endpoint)
            , access_policy_
            }
  {}

  template< is_protocol Protocol
          , is_access_policy AccessPolicy
          , is_command... Commands
          >
    template<typename Executor>
      ClientState<Protocol, AccessPolicy, Commands...>::ClientState
        ( Executor& executor
        , util::ASIO::Connectable<Protocol> connectable
        , std::shared_ptr<AccessPolicy> access_policy_
        )
          : ClientState<Protocol, AccessPolicy, Commands...>
            { detail::connected_socket<Protocol, Commands...>
               (executor, connectable)
            , access_policy_
            }
  {}
}
