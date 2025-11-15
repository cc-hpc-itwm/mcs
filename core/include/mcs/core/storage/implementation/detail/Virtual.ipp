// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto Virtual::chunk_description
      ( Parameter::Chunk::Description parameter_chunk_description
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  {
    return _imported_c_api.template chunk_description<Access>
      ( parameter_chunk_description
      , segment_id
      , memory_range
      );
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Virtual::Error::Create::parameter
    (
    ) const noexcept -> Parameter::Create const&
  {
    return _parameter;
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "mcs::core::storage::implementation::Virtual"
  , mcs::core::storage::implementation::Virtual::Tag
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "Virtual "
  , mcs::core::storage::implementation::Virtual::Parameter::Create
  , _shared_object
  , _parameter_create
  );
