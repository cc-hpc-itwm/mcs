// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::core::transport
{
  auto make_address
    ( storage::ID storage_id
    , storage::Parameter storage_parameter_chunk_description
    , storage::segment::ID segment_id
    , memory::Offset offset
    ) -> Address
  {
    return Address
      { .storage_id = storage_id
      , .storage_parameter_chunk_description =
          std::move (storage_parameter_chunk_description)
      , .segment_id = segment_id
      , .offset = offset
      };
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::core::transport::Address>::output
    ( OArchive& oa
    , mcs::core::transport::Address const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.storage_parameter_chunk_description);
    save (oa, value.segment_id);
    save (oa, value.offset);

    return oa;
  }
  auto Implementation<mcs::core::transport::Address>::input
    ( IArchive& ia
    ) -> mcs::core::transport::Address
  {
    auto storage_id {load<decltype (mcs::core::transport::Address::storage_id)> (ia)};
    auto storage_parameter_chunk_description {load<decltype (mcs::core::transport::Address::storage_parameter_chunk_description)> (ia)};
    auto segment_id {load<decltype (mcs::core::transport::Address::segment_id)> (ia)};
    auto offset {load<decltype (mcs::core::transport::Address::offset)> (ia)};

    return mcs::core::transport::make_address
      ( storage_id
      , std::move (storage_parameter_chunk_description)
      , segment_id
      , offset
      );
  }
}
