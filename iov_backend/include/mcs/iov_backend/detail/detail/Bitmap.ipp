// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cassert>

namespace mcs::iov_backend::detail
{
  constexpr auto Bitmap::size() const noexcept -> Index
  {
    return _size;
  }

  template<typename TimePoint>
    auto Bitmap::set
      ( InterruptionContext const& interruption_context
      , TimePoint const& time_point
      ) -> Index
  {
    auto lock {std::unique_lock {_guard}};

    _bit_cleared_or_interrupted_or_error.wait_until
      ( lock
      , time_point
      , [&]
        {
          return _error
            || interruption_context._interrupted
            || _clear
            || find_clear()
            ;
        }
      );

    if (_error)
    {
      std::rethrow_exception (_error);
    }

    if (interruption_context._interrupted)
    {
      throw Error::Set::Interrupted{};
    }

    if (!_clear)
    {
      throw Error::Set::Timeout{};
    }

    set();

    return index();
  }

  constexpr auto Bitmap::index() const noexcept -> Index
  {
    return _slot * _bits_per_entry + _bit;
  }

  constexpr auto Bitmap::mask() const noexcept -> Entry
  {
    return Entry {1} << _bit;
  }

  constexpr auto Bitmap::test() const noexcept -> Entry
  {
    return _entries[_slot] & mask();
  }

  constexpr auto Bitmap::find_clear() -> bool
  {
    auto slots_to_test {_slots};

    while (slots_to_test --> 0)
    {
      if (! (++_bit < _bits_per_entry))
      {
        _bit = 0;
        if (! (++_slot < _slots))
        {
          _slot = 0;
        }
      }

      if (index() < _size)
      {
        if (test() == 0)
        {
          return _clear = true;
        }
      }
    }

    return _clear = false;
  }

  constexpr auto Bitmap::set() noexcept -> void
  {
    assert (test() == 0);

    _entries[_slot] |= mask();
    _clear = false;
  }
  constexpr auto Bitmap::clear() noexcept -> void
  {
    assert (test() != 0);

    _entries[_slot] &= ~mask();
    _clear = true;
  }
}
