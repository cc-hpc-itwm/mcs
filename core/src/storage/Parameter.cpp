// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::core::storage
{
  Parameter::Parameter (std::vector<std::byte> blob)
    : _blob {blob}
  {}
}

namespace mcs::serialization
{
  auto Implementation<mcs::core::storage::Parameter>::output
    ( OArchive& oa
    , mcs::core::storage::Parameter const& value
    ) -> OArchive&
  {
    save (oa, value._blob);

    return oa;
  }
  auto Implementation<mcs::core::storage::Parameter>::input
    ( IArchive& ia
    ) -> mcs::core::storage::Parameter
  {
    auto _blob {load<decltype (mcs::core::storage::Parameter::_blob)> (ia)};

    return mcs::core::storage::Parameter {_blob};
  }
}
