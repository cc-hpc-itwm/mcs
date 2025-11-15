// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/block_device/block/Count.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/block_device/meta_data/Commands.hpp>
#include <mcs/block_device/meta_data/command/BlockSize.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::block_device::meta_data
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    struct Client : public Commands::template wrap< rpc::Client
                                                  , Protocol
                                                  , AccessPolicy
                                                  >
  {
    using Base = Commands::template wrap< rpc::Client
                                        , Protocol
                                        , AccessPolicy
                                        >;

    template<typename Executor>
      Client ( Executor&
             , util::ASIO::Connectable<Protocol>
             );

    // \note: documentation in class Blocks
    [[nodiscard]] auto constexpr block_size() const noexcept -> block::Size;
    [[nodiscard]] auto number_of_blocks() const -> block::Count;
    [[nodiscard]] auto blocks() const -> std::list<block::Range>;
    [[nodiscard]] auto add (Storage) const -> Blocks::AddResult;
    auto remove (block::Range) const -> Blocks::RemoveResult;
    [[nodiscard]] auto location (block::ID) const -> Blocks::Location;

  private:
    block::Size _block_size {Base::operator() (command::BlockSize{})};
  };
}

#include "detail/Client.ipp"
