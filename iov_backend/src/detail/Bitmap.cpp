// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cassert>
#include <mcs/iov_backend/detail/Bitmap.hpp>

namespace mcs::iov_backend::detail
{
  Bitmap::Bitmap (Index size)
    : _size {size}
  {
    std::ranges::fill (_entries, Entry {0});
  }

  auto Bitmap::clear (Index index) -> void
  {
    assert (index < size());

    {
      auto const lock {std::lock_guard {_guard}};

      if (_error)
      {
        std::rethrow_exception (_error);
      }

      _slot = index / _bits_per_entry;
      _bit = index % _bits_per_entry;

      clear();
    }

    _bit_cleared_or_interrupted_or_error.notify_all();
  }

  auto Bitmap::interrupt
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

    _bit_cleared_or_interrupted_or_error.notify_all();
  }

  auto Bitmap::error (std::exception_ptr error) -> void
  {
    {
      auto const lock {std::lock_guard {_guard}};

      if (_error)
      {
        std::rethrow_exception (_error);
      }

      _error = error;
    }

    _bit_cleared_or_interrupted_or_error.notify_all();
  }

  Bitmap::Error::Set::Interrupted::Interrupted()
    : mcs::Error {"iov_backend::Bitmap::Set::Interrupted"}
  {}
  Bitmap::Error::Set::Interrupted::~Interrupted() = default;

  Bitmap::Error::Set::Timeout::Timeout()
    : mcs::Error {"iov_backend::Bitmap::Set::Timeout"}
  {}
  Bitmap::Error::Set::Timeout::~Timeout() = default;
}
