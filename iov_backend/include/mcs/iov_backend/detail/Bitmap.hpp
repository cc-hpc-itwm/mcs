// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <algorithm>
#include <bit>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <iterator>
#include <limits>
#include <mcs/Error.hpp>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <variant>

namespace mcs::iov_backend::detail
{
  // thread safe, blocking get, interruption context per call, strong
  // exception safe
  //
  struct Bitmap
  {
    using Index = std::size_t;

    // Creates a Bitmap for size many bits.
    //
    // Post: All bits are clear()-ed.
    //
    // Complexity: O(size)
    //
    explicit Bitmap (Index size);

    // Returns: The number of bits in the map.
    //
    // Complexity: O(1)
    //
    [[nodiscard]] constexpr auto size() const noexcept -> Index;

    // Clears the bit with the given index.
    //
    // Expects: index < size()
    // Expects: The bit at index is set.
    // Ensures: The bit at index is clear.
    //
    // Complexity: O(1)
    //
    auto clear (Index) -> void;

    // Context to be able to interrupt call to get()
    //
    struct InterruptionContext
    {
      constexpr explicit InterruptionContext() noexcept = default;

    private:
      friend struct Bitmap;

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
      struct Set
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

    // Returns: The index of a clear bit.
    //
    // Ensures: The bit at the returned index is set.
    //
    // Note: The function blocks if all bits are set. It unblocks if a
    // bit is unset, if the parameter time_point has been exceeded or
    // if it is interrupted via the interruption context.
    // If the function reaches the timeout, then "Timeout" is thrown.
    // If the function is interrupted, then "Interrupted" is thrown.
    //
    template<typename TimePoint>
      [[nodiscard]] auto set
        ( InterruptionContext const&
        , TimePoint const&
        ) -> Index;

    // Marks the bitmap as failed. Sticky: All future usage will throw
    // the error.
    // Post: all blocked set() throw this error
    // Post: all future set() throw this error
    // Post: all future unset() throw this error
    // Post: all future error() throw this error
    // Post: all future interrupt() throw this error
    //
    auto error (std::exception_ptr) -> void;

  private:
    using Entry = std::uintmax_t;
    static constexpr Index _bits_per_entry {sizeof (Entry) * CHAR_BIT};

    Index _size;
    Index _slots {(_size + _bits_per_entry + 1) / _bits_per_entry};

    std::mutex _guard;
    std::condition_variable _bit_cleared_or_interrupted_or_error;
    std::unique_ptr<Entry[]> _storage
      { std::make_unique_for_overwrite<Entry[]> (_slots)
      };
    std::span<Entry> _entries {_storage.get(), _storage.get() + _slots};
    std::exception_ptr _error{};

    Index _slot {0};
    Index _bit {0};
    bool _clear {_size > 0};

    [[nodiscard]] constexpr auto index() const noexcept -> Index;
    [[nodiscard]] constexpr auto mask() const noexcept -> Entry;
    [[nodiscard]] constexpr auto test() const noexcept -> Entry;
    [[nodiscard]] constexpr auto find_clear() -> bool;
    constexpr auto set() noexcept -> void;
    constexpr auto clear() noexcept -> void;
  };
}

#include "detail/Bitmap.ipp"
