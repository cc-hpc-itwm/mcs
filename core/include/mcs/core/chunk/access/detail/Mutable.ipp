// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::core::chunk::access
{
  template<typename T>
    constexpr Mutable::Value<T>::Value (T value)
      : _value {std::move (value)}
  {}
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::chunk::access::Mutable>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::chunk::access::Mutable>::format
      ( mcs::core::chunk::access::Mutable const& /* access */
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "chunk::access::Mutable");
  }
}
