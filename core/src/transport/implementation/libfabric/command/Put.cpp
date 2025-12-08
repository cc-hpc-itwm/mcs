// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/command/Put.hpp>
#include <mcs/serialization/define.hpp>

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , put
    , core::transport::implementation::libfabric::command::Put
    )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, put, destination);
    MCS_SERIALIZATION_SAVE_FIELD (oa, put, size);

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , core::transport::implementation::libfabric::command::Put
    )
  {
    namespace libfabric = core::transport::implementation::libfabric;
    using Put = libfabric::command::Put;

    MCS_SERIALIZATION_LOAD_FIELD (ia, destination, Put);
    MCS_SERIALIZATION_LOAD_FIELD (ia, size, Put);

    return Put {destination, size};
  }
}
