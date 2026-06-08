// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <future>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/State.hpp>
#include <mcs/fuse/detail/managed_not_null_resource.hpp>
#include <mcs/fuse/detail/state_change.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/session/Options.hpp>
#include <mcs/fuse/state/operations.hpp>
#include <mcs/util/member_AUTO.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <thread>
#include <variant>

namespace mcs::fuse
{
  template<is_content Content>
    struct Session
  {
    // Starts the FUSE event loop on a thread owned by this Session.
    //
    [[nodiscard]] Session ( util::not_null<::fuse_args>
                          , util::not_null<typename Content::State>
                          , session::Options
                          )
      ;

    // The returned future becomes ready when the loop
    // returns. Multiple calls return the same shared future.
    //
    [[nodiscard]] auto result() const -> std::shared_future<int>;

    Session (Session const&) = delete;
    Session (Session&&) = delete;
    auto operator= (Session const&) -> Session& = delete;
    auto operator= (Session&&) -> Session& = delete;
    ~Session();

  private:
    util::not_null<::fuse_args> _args;
    util::not_null<typename Content::State> _content_state;
    session::Options _options;

    MCS_UTIL_MEMBER_AUTO (_state, State<Content> {_content_state});
    MCS_UTIL_MEMBER_AUTO (_operations, state::operations<Content>());

    MCS_UTIL_MEMBER_AUTO
      ( _session
      , detail::managed_not_null_resource
        ( ::fuse_session_destroy
        , "fuse_session_new"
        , ::fuse_session_new_fn
        , _args.get()
        , std::addressof (_operations)
        , sizeof (_operations)
        , std::addressof (_state)
        )
      );

    // Lambdas can not capture _session or _options.
    // NOLINTBEGIN  (modernize-avoid-bind)
    //
    MCS_UTIL_MEMBER_AUTO
      ( _fuse_signal_handlers
      , detail::state_change
        ( std::bind (::fuse_remove_signal_handlers, _session.get())
        , "fuse_set_signal_handlers"
        , std::bind (::fuse_set_signal_handlers, _session.get())
        )
      );

    MCS_UTIL_MEMBER_AUTO
      ( _fuse_mount
      , detail::state_change
        ( std::bind (::fuse_session_unmount, _session.get())
        , "fuse_session_mount"
        , std::bind
          ( ::fuse_session_mount
          , _session.get()
          , _options.mountpoint()
          )
        )
      );
    //
    // NOLINTEND  (modernize-avoid-bind)

    // Keep _loop last: reverse destruction joins _thread before any
    // of the members above (state, operations, session, signal
    // handlers, mount) are destroyed.
    //
    struct Loop
    {
      Loop (session::Options const&, ::fuse_session*);

      Loop (Loop const&) = delete;
      Loop (Loop&&) = delete;
      auto operator= (Loop const&) -> Loop& = delete;
      auto operator= (Loop&&) -> Loop& = delete;
      ~Loop();

      [[nodiscard]] auto result() const -> std::shared_future<int>;

    private:
      Loop (std::packaged_task<int()>);

      std::shared_future<int> _result{};
      std::jthread _thread{};
    };
    Loop _loop {_options, _session.get()};
  };
}

#include "detail/Session.ipp"
