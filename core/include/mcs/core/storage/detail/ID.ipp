// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/prefix.hpp>

namespace mcs::core::storage
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

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::ID>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::ID>::format
      ( mcs::core::storage::ID const& id
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "bi_{}", id._value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<core::storage::ID>::read
      ( State<Char>& state
      ) -> core::storage::ID
  {
    maybe_prefix (state, "bi_");

    return core::storage::ID
      {parse<core::storage::ID::underlying_type> (state)};
  }
}
