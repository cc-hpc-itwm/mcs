// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdlib>
#include <fmt/base.h>
#include <mcs/fuse/Session.hpp>
#include <mcs/fuse/detail/managed_object.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <memory>

namespace mcs::fuse
{
  template<is_content Content, typename PrintUsage>
    auto main ( PrintUsage print_usage
              , int argc
              , char** argv
              , typename Content::State content_state
              ) -> int
  {
    auto const args
      { detail::managed_object<::fuse_args>
        ( ::fuse_opt_free_args
        , argc
        , argv
        , 0
        )
      };

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

    return Session<Content>
      { args.get()
      , std::addressof (content_state)
      , session::Options {options.get()}
      }.result().get()
      ;
  }
}
