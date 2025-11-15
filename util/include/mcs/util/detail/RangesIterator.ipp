// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/Error.hpp>
#include <memory>

namespace mcs::util
{
  template<typename Ranges>
    constexpr RangesIterator<Ranges>::RangesIterator (Ranges const& ranges)
      : _pos {std::cbegin (ranges)}
      , _end {std::cend (ranges)}
  {
    if (_pos != _end)
    {
      _value = begin (*_pos);
    }
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator*() const -> reference
  {
    if (!_value.has_value())
    {
      throw mcs::Error {"RangesIterator::operator*: no value"};
    }

    return *_value;
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator->() const -> pointer
  {
    if (!_value.has_value())
    {
      throw mcs::Error {"RangesIterator::operator->: no value"};
    }

    return std::addressof (*_value);
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator++() -> RangesIterator&
  {
    if (!_value.has_value())
    {
      throw mcs::Error {"RangesIterator::operator++: no value"};
    }

    if (++*_value == end (*_pos))
    {
      if (++_pos != _end)
      {
        _value = begin (*_pos);
      }
      else
      {
       _value.reset();
      }
    }

    return *this;
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator++ (int) -> RangesIterator
  {
    auto old {RangesIterator {*this}};
    this->operator++();
    return old;
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator==
      ( RangesIterator const& other
      ) const -> bool
  {
    return (!_value.has_value() && !other._value.has_value())
      || _pos == other._pos
      ;
  }

  template<typename Ranges>
    constexpr auto RangesIterator<Ranges>::operator!=
      ( RangesIterator const& other
      ) const -> bool
  {
    return !this->operator== (other);
  }

  template<typename Ranges>
    constexpr RangesIterator<Ranges>::operator bool() const
  {
    return _value.has_value();
  }
}
