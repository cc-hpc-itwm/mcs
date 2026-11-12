// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/base.h>
#include <fmt/format.h>
#include <mcs/fuse/content/implementation/storage/for_each_part.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <stdexcept>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto file {Parameter {"file"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PATH>\n"
        "    File whose storage distribution is inspected.\n"
      , args.at (0)
      , option::file
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto mcs_distribution_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::file)
        )
      };

    mcs::fuse::content::storage::for_each_part
      ( options.at (option::file)
      , [] (auto const& part)
        {
          fmt::print ("{}\n", part);
        }
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, mcs_distribution_main);
}
