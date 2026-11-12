// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/prefix.hpp>

namespace mcs::core::storage::segment
{
  constexpr ID::ID (underlying_type value) noexcept
    : _value {value}
  {}

  constexpr auto ID::operator++() noexcept -> ID&
  {
    ++_value;

    return *this;
  }
}

namespace mcs::util
{
  constexpr auto Cast< core::storage::segment::ID
                     , ::mcs_core_storage_segment_id
                     >::operator()
    ( ::mcs_core_storage_segment_id id
    ) const -> core::storage::segment::ID
  {
    return core::storage::segment::ID {id.value};
  }

  constexpr auto Cast< ::mcs_core_storage_segment_id
                     , core::storage::segment::ID
                     >::operator()
    ( core::storage::segment::ID id
    ) const -> ::mcs_core_storage_segment_id
  {
    return ::mcs_core_storage_segment_id {id._value};
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::segment::ID>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::segment::ID>::format
      ( mcs::core::storage::segment::ID const& id
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "sg_{}", id._value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::storage::segment::ID>::read
      ( State<Char>& state
      ) -> core::storage::segment::ID
  {
    maybe_prefix (state, "sg_");

    return core::storage::segment::ID
      {parse<core::storage::segment::ID::underlying_type> (state)};
  }
}
