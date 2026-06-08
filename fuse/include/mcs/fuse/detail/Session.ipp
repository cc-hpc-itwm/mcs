// Copyright (C) 2025-2026 Fraunhofer ITWM
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
    Session<Content>::Loop::Loop
      ( session::Options const& options
      , ::fuse_session* session
      )
        : Loop
          { std::packaged_task<int()>
            { [options, session]
              {
                ::fuse_daemonize (options.foreground());

                if (options.singlethread())
                {
                  return ::fuse_session_loop (session);
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
                      , options.clone_fd()
                      );
                  ::fuse_loop_cfg_set_max_threads
                      ( config.get()
                      , options.max_threads()
                      );

                  return ::fuse_session_loop_mt (session, config.get());
                }
              }
            }
          }
  {}
  template<is_content Content>
    Session<Content>::Loop::Loop (std::packaged_task<int()> task)
      : _result {task.get_future().share()}
      , _thread {std::move (task)}
  {}
  template<is_content Content>
    auto Session<Content>::Loop::result() const -> std::shared_future<int>
  {
    return _result;
  }
  template<is_content Content>
    Session<Content>::Loop::~Loop()
  {
    _thread.join();
  }

  template<is_content Content>
    auto Session<Content>::result() const -> std::shared_future<int>
  {
    return _loop.result();
  }
  template<is_content Content>
    Session<Content>::~Session()
  {
    ::fuse_session_exit (_session.get());
  }
}
