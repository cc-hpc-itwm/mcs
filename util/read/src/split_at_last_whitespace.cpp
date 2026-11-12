// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/util/isspace.hpp>
#include <mcs/util/read/split_at_last_whitespace.hpp>
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace mcs::util::read
{
  auto split_at_last_whitespace (std::string_view text) -> SeparatedFields
  {
    auto const rev_text {text | std::views::reverse};

    if ( auto const rev_pos {std::ranges::find_if (rev_text, isspace<char>)}
       ; rev_pos != std::end (rev_text)
       )
    {
      if ( auto const pos {rev_pos.base()}
         ; pos != std::end (text) && std::prev (pos) != std::begin (text)
         )
      {
        return SeparatedFields
          { .prefix = std::string_view {std::begin (text), std::prev (pos)}
          , .suffix = std::string_view {pos, std::end (text)}
          };
      }
    }

    throw std::invalid_argument
      { "expected two nonempty fields separated by whitespace"
      };
  }
}
