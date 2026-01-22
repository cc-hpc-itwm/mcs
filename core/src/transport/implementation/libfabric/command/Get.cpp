// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/command/Get.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<core::transport::implementation::libfabric::command::Get>::output
    ( OArchive& oa
    , core::transport::implementation::libfabric::command::Get const& get
    ) -> OArchive&
  {
    save (oa, get.source);
    save (oa, get.size);
    save (oa, get.name);

    return oa;
  }

  auto Implementation<core::transport::implementation::libfabric::command::Get>::input
    ( IArchive& ia
    ) -> core::transport::implementation::libfabric::command::Get
  {
    namespace libfabric = core::transport::implementation::libfabric;
    using Get = libfabric::command::Get;

    auto source {load<decltype (Get::source)> (ia)};
    auto size {load<decltype (Get::size)> (ia)};
    auto name {load<decltype (Get::name)> (ia)};

    return Get {source, size, name};
  }
}
