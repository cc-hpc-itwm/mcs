// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::block_device::meta_data
{
  template<rpc::is_protocol Protocol>
    template<typename Executor>
      Provider<Protocol>::Provider
        ( typename Protocol::endpoint endpoint
        , Executor& executor
        , util::not_null<Blocks> blocks
        )
      : _provider {endpoint, executor, blocks}
  {}

  template<rpc::is_protocol Protocol>
    auto Provider<Protocol>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _provider.local_endpoint();
  }
}
