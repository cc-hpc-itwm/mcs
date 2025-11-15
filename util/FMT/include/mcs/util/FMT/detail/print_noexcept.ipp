// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/ostream.h>
#include <tuple>

namespace mcs::util::FMT
{
  template<fmt::formattable... Args>
    constexpr auto print_noexcept
      ( util::FMT::sink auto sink
      , fmt::format_string<Args...> format
      , Args&&... args
      ) noexcept -> void
  try
  {
    fmt::print (sink, format, std::forward<Args> (args)...);
  }
  catch (...)
  {
    std::ignore = std::current_exception();
  }
}
