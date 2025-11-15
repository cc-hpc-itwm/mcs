// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/meta_data/provider/Handler.hpp>

namespace mcs::block_device::meta_data::provider
{
  Handler::Handler (util::not_null<Blocks> blocks)
    : _blocks {blocks}
  {}

  auto Handler::operator()
    ( command::BlockSize
    ) const -> command::BlockSize::Response
  {
    return _blocks->block_size();
  }

  auto Handler::operator()
    ( command::NumberOfBlocks
    ) const -> command::NumberOfBlocks::Response
  {
    return _blocks->number_of_blocks();
  }

  auto Handler::operator() (command::Blocks) const -> command::Blocks::Response
  {
    return _blocks->blocks();
  }

  auto Handler::operator() (command::Add add) -> command::Add::Response
  {
    return _blocks->add (add.storage);
  }

  auto Handler::operator() (command::Remove remove) -> command::Remove::Response
  {
    return _blocks->remove (remove.range);
  }

  auto Handler::operator()
    ( command::Location location
    ) const -> command::Location::Response
  {
    return _blocks->location (location.id);
  }
}
