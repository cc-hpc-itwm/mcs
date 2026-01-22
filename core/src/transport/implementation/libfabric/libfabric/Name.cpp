// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  Name::Name (std::vector<std::byte> value) noexcept
    : _value {std::move (value)}
  {}

  auto Name::value() const noexcept -> std::vector<std::byte> const&
  {
    return _value;
  }
  Name::operator std::vector<std::byte> const&() const noexcept
  {
    return _value;
  }
}

namespace mcs::serialization
{
  auto Implementation<core::transport::implementation::libfabric::libfabric::Name>::output
    ( OArchive& oa
    , core::transport::implementation::libfabric::libfabric::Name const& name
    ) -> OArchive&
  {
    save (oa, name._value);

    return oa;
  }
  auto Implementation<core::transport::implementation::libfabric::libfabric::Name>::input
    ( IArchive& ia
    ) -> core::transport::implementation::libfabric::libfabric::Name
  {
    using Name
      = core::transport::implementation::libfabric::libfabric::Name
      ;

    auto _value {load<decltype (Name::_value)> (ia)};

    return Name {std::move (_value)};
  }
}
