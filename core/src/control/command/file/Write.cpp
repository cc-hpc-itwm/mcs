// Copyright (C) 2024-2025 Fraunhofer ITWM
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
    save (oa, value._storage_id);
    save (oa, value._parameter_file_write);
    save (oa, value._segment_id);
    save (oa, value._offset);
    save (oa, value._file);
    save (oa, value._range);

    return oa;
  }
  auto Implementation<mcs::core::control::command::file::Write>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::file::Write
  {
    auto _storage_id {load<decltype (mcs::core::control::command::file::Write::_storage_id)> (ia)};
    auto _parameter_file_write {load<decltype (mcs::core::control::command::file::Write::_parameter_file_write)> (ia)};
    auto _segment_id {load<decltype (mcs::core::control::command::file::Write::_segment_id)> (ia)};
    auto _offset {load<decltype (mcs::core::control::command::file::Write::_offset)> (ia)};
    auto _file {load<decltype (mcs::core::control::command::file::Write::_file)> (ia)};
    auto _range {load<decltype (mcs::core::control::command::file::Write::_range)> (ia)};

    return mcs::core::control::command::file::Write {_storage_id, _parameter_file_write, _segment_id, _offset, _file, _range};
  }
}
