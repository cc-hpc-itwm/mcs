// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/define.hpp>
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
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, name, core::transport::implementation::libfabric::libfabric::Name)
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, name, _value);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, core::transport::implementation::libfabric::libfabric::Name)
  {
    using Name
      = core::transport::implementation::libfabric::libfabric::Name
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _value, Name);

    return Name {std::move (_value)};
  }
}
