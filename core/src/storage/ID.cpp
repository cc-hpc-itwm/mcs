// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/storage/ID.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::storage::ID>::output
    ( OArchive& oa
    , mcs::core::storage::ID const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::core::storage::ID>::input
    ( IArchive& ia
    ) -> mcs::core::storage::ID
  {
    auto _value {load<decltype (mcs::core::storage::ID::_value)> (ia)};

    return mcs::core::storage::ID {_value};
  }
}

namespace std
{
  auto hash<mcs::core::storage::ID>::operator()
    ( mcs::core::storage::ID x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}
