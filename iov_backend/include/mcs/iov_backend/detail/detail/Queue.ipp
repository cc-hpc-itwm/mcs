// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::iov_backend::detail
{
  template<typename T>
    template<typename... Args>
      requires (std::is_constructible_v<T, Args...>)
      auto Queue<T>::push (Args&&... args) -> Queue&
  {
    {
      auto const lock {std::lock_guard {_guard}};

      if (_error)
      {
        std::rethrow_exception (_error);
      }

      _elements.emplace_back (std::forward<Args> (args)...);
    }

    _element_added_or_interrupted_or_error.notify_all();

    return *this;
  }

  template<typename T>
    auto Queue<T>::interrupt
      ( InterruptionContext& interruption_context
      ) -> void
  {
    {
      auto const lock {std::lock_guard {_guard}};

      if (_error)
      {
        std::rethrow_exception (_error);
      }

      interruption_context._interrupted = true;
    }

    _element_added_or_interrupted_or_error.notify_all();
  }

  template<typename T>
    auto Queue<T>::error (std::exception_ptr error) -> void
  {
    {
      auto const lock {std::lock_guard {_guard}};

      if (_error)
      {
        std::rethrow_exception (_error);
      }

      _error = error;
    }

    _element_added_or_interrupted_or_error.notify_all();
  }

  template<typename T>
    Queue<T>::Error::Get::Interrupted::Interrupted()
      : mcs::Error {"iov_backend::Queue::Get::Interrupted"}
  {}
  template<typename T>
    Queue<T>::Error::Get::Interrupted::~Interrupted() = default;

  template<typename T>
    Queue<T>::Error::Get::Timeout::Timeout()
      : mcs::Error {"iov_backend::Queue::Get::Timeout"}
  {}
  template<typename T>
    Queue<T>::Error::Get::Timeout::~Timeout() = default;

  template<typename T>
    template<typename TimePoint>
      auto Queue<T>::get
        ( InterruptionContext const& interruption_context
        , TimePoint const& time_point
        ) -> T
  {
    auto lock {std::unique_lock {_guard}};

    _element_added_or_interrupted_or_error.wait_until
      ( lock
      , time_point
      , [&]
        {
          return _error
            || interruption_context._interrupted
            || !_elements.empty()
            ;
        }
      );

    if (_error)
    {
      std::rethrow_exception (_error);
    }

    if (interruption_context._interrupted)
    {
      throw typename Error::Get::Interrupted{};
    }

    if (_elements.empty())
    {
      throw typename Error::Get::Timeout{};
    }

    auto head {std::move (_elements.front())};
    _elements.pop_front();
    return head;
  }
}
