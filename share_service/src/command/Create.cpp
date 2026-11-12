// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/share_service/command/Create.hpp>
#include <utility>

namespace mcs::share_service::command
{
  auto make_create
    ( core::memory::Size size
    , Create::Parameters::Variant parameters
    ) -> Create
  {
    return Create {.size = size, .parameters = std::move (parameters)};
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::share_service::command::Create>::output
    ( OArchive& oa
    , mcs::share_service::command::Create const& value
    ) -> OArchive&
  {
    save (oa, value.size);
    save (oa, value.parameters);

    return oa;
  }
  auto Implementation<mcs::share_service::command::Create>::input
    ( IArchive& ia
    ) -> mcs::share_service::command::Create
  {
    auto size {load<decltype (mcs::share_service::command::Create::size)> (ia)};
    auto parameters {load<decltype (mcs::share_service::command::Create::parameters)> (ia)};

    return mcs::share_service::command::make_create
      ( size
      , std::move (parameters)
      );
  }
}
