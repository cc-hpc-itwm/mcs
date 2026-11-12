// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/file/Write.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::file::Write>::output
    ( OArchive& oa
    , mcs::core::control::command::file::Write const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.parameter_file_write);
    save (oa, value.segment_id);
    save (oa, value.offset);
    save (oa, value.file);
    save (oa, value.range);

    return oa;
  }
  auto Implementation<mcs::core::control::command::file::Write>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::file::Write
  {
    auto storage_id {load<decltype (mcs::core::control::command::file::Write::storage_id)> (ia)};
    auto parameter_file_write {load<decltype (mcs::core::control::command::file::Write::parameter_file_write)> (ia)};
    auto segment_id {load<decltype (mcs::core::control::command::file::Write::segment_id)> (ia)};
    auto offset {load<decltype (mcs::core::control::command::file::Write::offset)> (ia)};
    auto file {load<decltype (mcs::core::control::command::file::Write::file)> (ia)};
    auto range {load<decltype (mcs::core::control::command::file::Write::range)> (ia)};

    return mcs::core::control::command::file::Write
      { .storage_id = storage_id
      , .parameter_file_write = parameter_file_write
      , .segment_id = segment_id
      , .offset = offset
      , .file = file
      , .range = range
      };
  }
}
