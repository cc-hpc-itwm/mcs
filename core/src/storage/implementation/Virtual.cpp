// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/core/storage/c_api.h>
#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/util/tuplish/define.hpp>

namespace mcs::core::storage::implementation
{
  Virtual::Virtual (Parameter::Create create)
  try
    : _dlhandle {create._shared_object}
    , _imported_c_api
      { Import_C_API::Parameter::Create
        { std::invoke
          ( MCS_UTIL_DLHANDLE_SYMBOL (_dlhandle, mcs_core_storage_methods)
          )
        , create._parameter_create
        }
      }
  {}
  catch (...)
  {
    std::throw_with_nested (Error::Create {create});
  }
}

namespace mcs::core::storage::implementation
{
  auto Virtual::size_max
    ( Parameter::Size::Max parameter_size_max
    ) const -> MaxSize
  {
    return _imported_c_api.size_max (parameter_size_max);
  }
}

namespace mcs::core::storage::implementation
{
  auto Virtual::size_used
    ( Parameter::Size::Used parameter_size_used
    ) const -> memory::Size
  {
    return _imported_c_api.size_used (parameter_size_used);
  }
}

namespace mcs::core::storage::implementation
{
  auto Virtual::segment_create
    ( Parameter::Segment::Create parameter_segment_create
    , memory::Size size
    ) -> segment::ID
  {
    return _imported_c_api.segment_create (parameter_segment_create, size);
  }
}

namespace mcs::core::storage::implementation
{
  auto Virtual::segment_remove
    ( Parameter::Segment::Remove parameter_segment_remove
    , segment::ID segment_id
    ) -> memory::Size
  {
    return _imported_c_api.segment_remove
      ( parameter_segment_remove
      , segment_id
      );
  }
}

namespace mcs::core::storage::implementation
{
  auto Virtual::file_read
    ( Parameter::File::Read parameter_file_read
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    return _imported_c_api.file_read
      ( parameter_file_read
      , segment_id
      , offset
      , path
      , range
      );
  }

  auto Virtual::file_write
    ( Parameter::File::Write parameter_file_write
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    return _imported_c_api.file_write
      ( parameter_file_write
      , segment_id
      , offset
      , path
      , range
      );
  }
}

namespace mcs::core::storage::implementation
{
  Virtual::Error::Create::Create
    ( Parameter::Create parameter
    )
      : mcs::Error
        { fmt::format ( "storage::Virtual::Create ({})"
                      , parameter
                      )
        }
      , _parameter {parameter}
  {}
  Virtual::Error::Create::~Create() = default;
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::storage::implementation::Virtual::Parameter::Create
  , _shared_object
  , _parameter_create
  );
