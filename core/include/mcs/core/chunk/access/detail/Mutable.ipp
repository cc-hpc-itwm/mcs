// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
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
  MCS_UTIL_FMT_DEFINE_PARSE (context, mcs::core::chunk::access::Mutable)
  {
    return context.begin();
  }

  MCS_UTIL_FMT_DEFINE_FORMAT
    ( /* access */
    , context
    , mcs::core::chunk::access::Mutable
    )
  {
    return fmt::format_to (context.out(), "chunk::access::Mutable");
  }
}
