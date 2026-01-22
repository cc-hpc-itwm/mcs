// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::core::chunk::access
{
  template<typename T>
    constexpr Const::Value<T>::Value (T value)
      : _value {std::move (value)}
  {}
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::chunk::access::Const>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::chunk::access::Const>::format
      ( mcs::core::chunk::access::Const const& /* access */
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "chunk::access::Const");
  }
}
