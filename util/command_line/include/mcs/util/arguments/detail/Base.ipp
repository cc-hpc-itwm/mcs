// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstddef>
#include <fmt/format.h>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <span>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace mcs::util::arguments::detail
{
  [[nodiscard]] auto is_top_level_argument
    ( std::stack<std::string_view>&
    , std::string_view
    ) -> bool;
}

namespace mcs::util::arguments
{
  template<typename Storage>
    constexpr Base<Storage>::Base (Storage storage)
      noexcept (std::is_nothrow_move_constructible_v<Storage>)
      : _storage {std::move (storage)}
  {}

  template<typename Storage>
    constexpr auto Base<Storage>::size() const noexcept -> std::size_t
  {
    return _storage.size();
  }

  template<typename Storage>
    constexpr auto Base<Storage>::at (std::size_t index) const -> char const*
  {
    if (index >= size())
    {
      throw std::out_of_range
        { fmt::format
          ( "Arguments::at: index {} out of range for {} arguments"
          , index
          , size()
          )
        };
    }

    return _storage[index];
  }

  template<typename Storage>
    constexpr auto Base<Storage>::begin() const noexcept -> Iterator
  {
    return std::begin (span());
  }

  template<typename Storage>
    constexpr auto Base<Storage>::end() const noexcept -> Iterator
  {
    return std::end (span());
  }

  template<typename Storage>
    auto Base<Storage>::contains_any_of
      ( std::initializer_list<std::string_view> values
      ) const -> bool
  {
    auto embeddings {std::stack<std::string_view>{}};

    return std::ranges::any_of
      ( _storage
      , [&] (std::string_view text)
        {
          return detail::is_top_level_argument (embeddings, text)
            && std::ranges::find (values, text) != values.end()
            ;
        }
      );
  }

  template<typename Storage>
    auto Base<Storage>::help_requested() const -> bool
  {
    return contains_any_of ({"-h", "--help"});
  }

  template<typename Storage>
    auto Base<Storage>::parse (command_line::Schema const& schema) const
      -> command_line::Parsed
  {
    return command_line::Parsed {span(), schema};
  }

  template<typename Storage>
    constexpr auto Base<Storage>::slice
      ( std::size_t offset
      ) const -> std::span<char const* const>
  {
    if (offset > size())
    {
      throw std::out_of_range
        { fmt::format
          ( "Arguments::from: offset {} out of range for {} arguments"
          , offset
          , size()
          )
        };
    }

    return span().subspan (offset);
  }

  template<typename Storage>
    constexpr auto Base<Storage>::span
      (
      ) const noexcept -> std::span<char const* const>
  {
    return std::span<char const* const> {_storage.data(), size()};
  }
}
