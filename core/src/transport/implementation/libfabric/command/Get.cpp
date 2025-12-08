// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/command/Get.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/serialization/define.hpp>

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , get
    , core::transport::implementation::libfabric::command::Get
    )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, get, source);
    MCS_SERIALIZATION_SAVE_FIELD (oa, get, size);
    MCS_SERIALIZATION_SAVE_FIELD (oa, get, name);

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , core::transport::implementation::libfabric::command::Get
    )
  {
    namespace libfabric = core::transport::implementation::libfabric;
    using Get = libfabric::command::Get;

    MCS_SERIALIZATION_LOAD_FIELD (ia, source, Get);
    MCS_SERIALIZATION_LOAD_FIELD (ia, size, Get);
    MCS_SERIALIZATION_LOAD_FIELD (ia, name, Get);

    return Get {source, size, name};
  }
}
