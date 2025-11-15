// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <condition_variable>
#include <deque>
#include <exception>
#include <mcs/Error.hpp>
#include <mutex>
#include <type_traits>
#include <variant>

namespace mcs::iov_backend::detail
{
  // FIFO, thread safe, blocking get, interruption context per call,
  // strong exception safe
  //
  template<typename T>
    struct Queue
  {
    // Creates a new element in place at the end of the queue. For
    // concurrent clients the "end" of the queue is not well defined,
    // their elements may be inserted in any order.
    //
    template<typename... Args>
      requires (std::is_constructible_v<T, Args...>)
      auto push (Args&&...) -> Queue&;

    // Context to be able to interrupt call to get()
    //
    struct InterruptionContext
    {
      constexpr explicit InterruptionContext() noexcept = default;

    private:
      friend struct Queue;

      bool _interrupted {false};
    };

    // Interrupt all ongoing calls that use this interruption context.
    //
    // Note: Stick: If an interruption contenxt has been interrupted,
    // then it will stay interrupted. Using it in a later call will
    // interrupt immediately.
    //
    auto interrupt (InterruptionContext&) -> void;

    struct Error
    {
      struct Get
      {
        struct Interrupted : public mcs::Error
        {
          Interrupted();
          MCS_ERROR_COPY_MOVE_DEFAULT (Interrupted);
        };
        struct Timeout : public mcs::Error
        {
          Timeout();
          MCS_ERROR_COPY_MOVE_DEFAULT (Timeout);
        };
      };
    };


    // Returns: The oldest element. "Oldest" is defined by the order
    // in which concurrent clients get the lock in push(). That is not
    // necessarily the same order as the calls to push() are started
    // or as the calls to push() are returning. For a single client an
    // element is "older" if it got pushed earlier.
    //
    // Note: The function blocks if no element is available. It
    // unblocks if the parameter time_point has been exceeded or if
    // it is interrupted via the interruption context.
    // If get is interrupted, then "Interrupted" is thrown.
    // if get reaches the timeout, then "Timeout" is thrown.
    //
    template<typename TimePoint>
      [[nodiscard]] auto get
        ( InterruptionContext const&
        , TimePoint const&
        ) -> T;

    // Marks the queue as failed. Sticky: All future usage will throw
    // the error.
    // Post: all blocked get() throw this error
    // Post: all future get() throw this error
    // Post: all future push() throw this error
    // Post: all future error() throw this error
    // Post: all future interrupt() throw this error
    //
    auto error (std::exception_ptr) -> void;

  private:
    std::mutex _guard;
    std::condition_variable _element_added_or_interrupted_or_error;
    std::deque<T> _elements;
    std::exception_ptr _error;
  };
}

#include "detail/Queue.ipp"
