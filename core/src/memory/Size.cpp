// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::memory::Size>::output
    ( OArchive& oa
    , mcs::core::memory::Size const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::core::memory::Size>::input
    ( IArchive& ia
    ) -> mcs::core::memory::Size
  {
    auto _value {load<decltype (mcs::core::memory::Size::_value)> (ia)};

    return mcs::core::memory::Size {_value};
  }
}

namespace std
{
  auto hash<mcs::core::memory::Size>::operator()
    ( mcs::core::memory::Size x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}
