// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/meta_data/Commands.hpp>
#include <memory>

namespace mcs::block_device::meta_data
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template<typename Executor>
      Client<Protocol, AccessPolicy>::Client
        ( Executor& executor
        , util::ASIO::Connectable<Protocol> connectable
        )
          : Base
            { executor
            , connectable
            , std::make_shared<AccessPolicy>()
            }
  {}

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto constexpr Client<Protocol, AccessPolicy>::block_size
      (
      ) const noexcept -> block::Size
  {
    return _block_size;
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::number_of_blocks
      (
      ) const -> block::Count
  {
    return Base::operator() (command::NumberOfBlocks{});
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::blocks
      (
      ) const -> std::list<block::Range>
  {
    return Base::operator() (command::Blocks{});
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::add
      ( Storage storage
      ) const -> Blocks::AddResult
  {
    return Base::operator() (command::Add {storage});
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::remove
      ( block::Range range
      ) const -> Blocks::RemoveResult
  {
    return Base::operator() (command::Remove {range});
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::location
      ( block::ID id
      ) const -> Blocks::Location
  {
    return Base::operator() (command::Location {id});
  }
}
