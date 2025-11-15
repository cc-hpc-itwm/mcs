// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <span>

namespace mcs::util
{
  struct Select
  {
    struct Error
    {
      struct OutOfRange : public mcs::Error
      {
      public:
        constexpr auto offset() const noexcept -> std::size_t;
        constexpr auto count() const noexcept -> std::size_t;
        constexpr auto size() const noexcept -> std::size_t;

        MCS_ERROR_COPY_MOVE_DEFAULT_IMPL (OutOfRange);
      private:
        friend struct Select;

        explicit OutOfRange
          ( std::size_t offset
          , std::size_t count
          , std::size_t size
          ) noexcept;

        std::size_t _offset;
        std::size_t _count;
        std::size_t _size;
      };
    };

    // A selection of a subspan that turns undefined behavior into an
    // exception.
    //
    // Expects: ! (offset + count > data.size())
    // Returns: data.subspan (offset, count)
    //
    template<typename T>
      friend constexpr auto select
        ( std::span<T> data
        , typename std::span<T>::size_type offset
        , typename std::span<T>::size_type count
        ) -> decltype (data.subspan (offset, count))
        ;

  private:
    template<typename T>
      static constexpr auto subspan
        ( std::span<T> data
        , typename std::span<T>::size_type offset
        , typename std::span<T>::size_type count
        ) -> decltype (data.subspan (offset, count))
      ;
  };
}

#include "detail/select.ipp"
