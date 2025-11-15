// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::share_service
{
  template<rpc::is_protocol Protocol>
    template<typename Executor>
      Provider<Protocol>::Provider
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        )
          : _provider {endpoint, executor, _storages}
  {}

  template<rpc::is_protocol Protocol>
    auto Provider<Protocol>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _provider.local_endpoint();
  }
}
