// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/syscall/raise.hpp>

namespace mcs::fuse
{
  template<is_content Content>
    Session<Content>::Session
      ( util::not_null<::fuse_args> args
      , util::not_null<typename Content::State> content_state
      , session::Options options
      )
    : _args {args}
    , _content_state {content_state}
    , _options {options}
  {}

  template<is_content Content>
    auto Session<Content>::run() const -> int
  {
    ::fuse_daemonize (_options.foreground());

    if (_options.singlethread())
    {
      return ::fuse_session_loop (_session.get());
    }
    else
    {
      auto const config
        { detail::managed_not_null_resource
          ( ::fuse_loop_cfg_destroy
          , "fuse_loop_cfg_create"
          , ::fuse_loop_cfg_create
          )
        };

      ::fuse_loop_cfg_set_clone_fd
          ( config.get()
          , _options.clone_fd()
          );
      ::fuse_loop_cfg_set_max_threads
          ( config.get()
          , _options.max_threads()
          );

      return ::fuse_session_loop_mt (_session.get(), config.get());
    }
  }

  template<is_content Content>
    Session<Content>::~Session()
  {
    ::fuse_session_exit (_session.get());
  }
}
