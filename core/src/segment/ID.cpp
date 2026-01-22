// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::storage::segment::ID>::output
    ( OArchive& oa
    , mcs::core::storage::segment::ID const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::core::storage::segment::ID>::input
    ( IArchive& ia
    ) -> mcs::core::storage::segment::ID
  {
    auto _value {load<decltype (mcs::core::storage::segment::ID::_value)> (ia)};

    return mcs::core::storage::segment::ID {_value};
  }
}

namespace std
{
  auto hash<mcs::core::storage::segment::ID>::operator()
    ( mcs::core::storage::segment::ID x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}
