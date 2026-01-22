// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/command/Put.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<core::transport::implementation::libfabric::command::Put>::output
    ( OArchive& oa
    , core::transport::implementation::libfabric::command::Put const& put
    ) -> OArchive&
  {
    save (oa, put.destination);
    save (oa, put.size);

    return oa;
  }

  auto Implementation<core::transport::implementation::libfabric::command::Put>::input
    ( IArchive& ia
    ) -> core::transport::implementation::libfabric::command::Put
  {
    namespace libfabric = core::transport::implementation::libfabric;
    using Put = libfabric::command::Put;

    auto destination {load<decltype (Put::destination)> (ia)};
    auto size {load<decltype (Put::size)> (ia)};

    return Put {destination, size};
  }
}
