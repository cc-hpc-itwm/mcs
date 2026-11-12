// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/arguments/Base.hpp>
#include <stack>
#include <stdexcept>
#include <string_view>

namespace mcs::util::arguments::detail
{
  auto is_top_level_argument
    ( std::stack<std::string_view>& embeddings
    , std::string_view text
    ) -> bool
  {
    if (text.starts_with ("[["))
    {
      embeddings.push (text.substr (2));

      return false;
    }

    if (  text.ends_with ("]]")
       && (!embeddings.empty() || !text.starts_with ("--"))
       )
    {
      if (embeddings.empty())
      {
        throw std::invalid_argument
          { fmt::format ("unmatched closing embedding tag '{}'", text)
          };
      }

      if (text.substr (0, text.size() - 2) != embeddings.top())
      {
        throw std::invalid_argument
          { fmt::format
            ( "mismatched closing embedding tag '{}'. expected '{}]]'"
            , text
            , embeddings.top()
            )
          };
      }

      embeddings.pop();

      return false;
    }

    return embeddings.empty();
  }
}
