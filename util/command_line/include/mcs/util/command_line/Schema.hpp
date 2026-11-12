// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <map>
#include <mcs/util/command_line/option/Flag.hpp>
#include <mcs/util/command_line/option/Parameter.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace mcs::util::command_line
{
  // Owns descriptors and registered embeddings, but borrows
  // descriptor names. Definitions are validated on parse.
  //
  struct Schema
  {
    Schema();

    auto add (option::Flag) & -> Schema&;
    auto add (option::Flag) && -> Schema&&;
    auto add (option::Parameter) & -> Schema&;
    auto add (option::Parameter) && -> Schema&&;
    auto add (Schema) & -> Schema&;
    auto add (Schema) && -> Schema&&;
    auto embedding (std::string) & -> Schema&;
    auto embedding (std::string) && -> Schema&&;

  private:
    using Definition = std::variant<option::Flag, option::Parameter>;
    using Embedded = std::map
                   < std::string
                   , std::vector<std::string_view>
                   , std::less<> // is_transparent
                   >;

    std::vector<Definition> _options{};
    Embedded _embeddings{};

    friend struct Parsed;
  };
}
