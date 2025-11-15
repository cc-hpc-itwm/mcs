// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>

namespace mcs::serialization
{
  template<core::storage::is_implementation Storage>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , create
      , core::storage::tracer::log_file::parameter::Create<Storage>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, create, _path);

    return oa;
  }

  template<core::storage::is_implementation Storage>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::tracer::log_file::parameter::Create<Storage>
      )
  {
    using Create
      = core::storage::tracer::log_file::parameter::Create<Storage>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _path, Create);

    return Create {_path};
  }
}
