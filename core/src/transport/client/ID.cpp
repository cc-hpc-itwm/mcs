// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/client/ID.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::transport::client::ID>::output
    ( OArchive& oa
    , mcs::core::transport::client::ID const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::core::transport::client::ID>::input
    ( IArchive& ia
    ) -> mcs::core::transport::client::ID
  {
    auto _value {load<decltype (mcs::core::transport::client::ID::_value)> (ia)};

    return mcs::core::transport::client::ID {_value};
  }
}

namespace std
{
  auto hash<mcs::core::transport::client::ID>::operator()
    ( mcs::core::transport::client::ID x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}
