// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  template<core::storage::is_implementation Storage>
    auto Implementation<core::storage::tracer::log_file::parameter::Create<Storage>>::output
      ( OArchive& oa
      , core::storage::tracer::log_file::parameter::Create<Storage> const& create
      ) -> OArchive&
  {
    save (oa, create._path);

    return oa;
  }

  template<core::storage::is_implementation Storage>
    auto Implementation<core::storage::tracer::log_file::parameter::Create<Storage>>::input
      ( IArchive& ia
      ) -> core::storage::tracer::log_file::parameter::Create<Storage>
  {
    using Create
      = core::storage::tracer::log_file::parameter::Create<Storage>
      ;

    auto _path {load<decltype (Create::_path)> (ia)};

    return Create {_path};
  }
}
