// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/memory/Offset.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::memory::Offset>::output
    ( OArchive& oa
    , mcs::core::memory::Offset const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::core::memory::Offset>::input
    ( IArchive& ia
    ) -> mcs::core::memory::Offset
  {
    auto _value {load<decltype (mcs::core::memory::Offset::_value)> (ia)};

    return mcs::core::memory::Offset {_value};
  }
}

namespace std
{
  auto hash<mcs::core::memory::Offset>::operator()
    ( mcs::core::memory::Offset x
    ) const noexcept -> size_t
  {
    return std::invoke (_hash, x._value);
  }
}
