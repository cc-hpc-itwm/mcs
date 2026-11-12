// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdlib>
#include <fmt/base.h>
#include <mcs/fuse/Session.hpp>
#include <mcs/fuse/detail/managed_object.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <memory>
#include <new>
#include <utility>

namespace mcs::fuse
{
  template<is_content Content, typename PrintUsage, typename Ready>
    auto main
      ( PrintUsage print_usage
      , util::arguments::View arguments
      , typename Content::State content_state
      , Ready ready
      ) -> int
  {
    auto const args
      { detail::managed_object<::fuse_args>
        ( ::fuse_opt_free_args
        )
      };

    std::ranges::for_each
      ( arguments
      , [&args] (char const* argument)
        {
          if (::fuse_opt_add_arg (args.get(), argument) != 0)
          {
            throw std::bad_alloc{};
          }
        }
      );

    auto const options
      { detail::managed_object<::fuse_cmdline_opts>
        ( [] (::fuse_cmdline_opts* _options)
          {
            free (_options->mountpoint);
          }
        )
      };

    if (::fuse_parse_cmdline (args.get(), options.get()) != 0)
    {
      print_usage();

      return EXIT_FAILURE;
    }

    if (options->show_help)
    {
      print_usage();

      return EXIT_SUCCESS;
    }

    if (options->show_version)
    {
      fmt::print ("FUSE library version {}\n", ::fuse_pkgversion());

      ::fuse_lowlevel_version();

      return EXIT_SUCCESS;
    }

    if (!options->mountpoint)
    {
      print_usage();

      return EXIT_FAILURE;
    }

    auto filesystem_session
      { Session<Content>
        { args.get()
        , std::addressof (content_state)
        , session::Options {options.get()}
        }
      };
    ready();
    return filesystem_session.result().get();
  }

  template<is_content Content, typename PrintUsage>
    auto main
      ( PrintUsage print_usage
      , util::arguments::View arguments
      , typename Content::State content_state
      ) -> int
  {
    return main<Content>
      ( std::move (print_usage)
      , arguments
      , std::move (content_state)
      , []() noexcept
        {
        }
      );
  }
}
