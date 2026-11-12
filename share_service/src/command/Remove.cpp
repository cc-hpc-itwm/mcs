// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/share_service/command/Remove.hpp>
#include <utility>

namespace mcs::share_service::command
{
  auto make_remove
    ( Chunk chunk
    , Remove::Parameters::Variant parameters
    ) -> Remove
  {
    return Remove
      { .chunk = std::move (chunk)
      , .parameters = std::move (parameters)
      };
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::share_service::command::Remove>::output
    ( OArchive& oa
    , mcs::share_service::command::Remove const& value
    ) -> OArchive&
  {
    save (oa, value.chunk);
    save (oa, value.parameters);

    return oa;
  }
  auto Implementation<mcs::share_service::command::Remove>::input
    ( IArchive& ia
    ) -> mcs::share_service::command::Remove
  {
    auto chunk {load<decltype (mcs::share_service::command::Remove::chunk)> (ia)};
    auto parameters {load<decltype (mcs::share_service::command::Remove::parameters)> (ia)};

    return mcs::share_service::command::make_remove
      ( std::move (chunk)
      , std::move (parameters)
      );
  }
}
