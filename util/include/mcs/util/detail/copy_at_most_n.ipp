// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util
{
  template<class InputIt, class Size, class OutputIt>
    constexpr auto copy_at_most_n
      ( InputIt first, InputIt last
      , Size count
      , OutputIt result
      ) -> OutputIt
  {
    for (; first != last && count --> 0; ++first, ++result)
    {
      *result = *first;
    }

    return result;
  }
}
