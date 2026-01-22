// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/storage/Size.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::storage::Size::Response>::output
    ( OArchive& oa
    , mcs::core::control::command::storage::Size::Response const& value
    ) -> OArchive&
  {
    save (oa, value._max);
    save (oa, value._used);

    return oa;
  }
  auto Implementation<mcs::core::control::command::storage::Size::Response>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::storage::Size::Response
  {
    auto _max {load<decltype (mcs::core::control::command::storage::Size::Response::_max)> (ia)};
    auto _used {load<decltype (mcs::core::control::command::storage::Size::Response::_used)> (ia)};

    return mcs::core::control::command::storage::Size::Response {_max, _used};
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::storage::Size>::output
    ( OArchive& oa
    , mcs::core::control::command::storage::Size const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.storage_parameter_size_max);
    save (oa, value.storage_parameter_size_used);

    return oa;
  }
  auto Implementation<mcs::core::control::command::storage::Size>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::storage::Size
  {
    auto storage_id {load<decltype (mcs::core::control::command::storage::Size::storage_id)> (ia)};
    auto storage_parameter_size_max {load<decltype (mcs::core::control::command::storage::Size::storage_parameter_size_max)> (ia)};
    auto storage_parameter_size_used {load<decltype (mcs::core::control::command::storage::Size::storage_parameter_size_used)> (ia)};

    return mcs::core::control::command::storage::Size {storage_id, storage_parameter_size_max, storage_parameter_size_used};
  }
}
