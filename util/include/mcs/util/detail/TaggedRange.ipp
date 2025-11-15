// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <iterator>
#include <map>
#include <ranges>
#include <unordered_map>

namespace mcs::util
{
  template<typename Tag, typename Range, typename Container>
    constexpr TaggedRange<Tag, Range, Container>::TaggedRange
      ( Tag tag
      , Range const& range
      )
  {
    for (auto x {std::cbegin (range)}; x != std::cend (range); ++x, ++tag)
    {
      _tagged.emplace (tag, x);
    }
  }

  template<typename Tag, typename Range, typename Container>
    constexpr auto TaggedRange<Tag, Range, Container>::tags() const noexcept
  {
    return _tagged | std::views::keys;
  }

  template<typename Tag, typename Range, typename Container>
    constexpr auto TaggedRange<Tag, Range, Container>::at (Tag const& tag) const
  {
    return *_tagged.at (tag);
  }
  template<typename Tag, typename Range>
    auto make_ordered_tagged_range (Tag tag, Range const& range)
  {
    using Value = decltype (std::cbegin (std::declval<Range>()));

    return TaggedRange<Tag, Range, std::map<Tag, Value>> (tag, range);
  }

  template<typename Tag, typename Range>
    auto make_unordered_tagged_range (Tag tag, Range const& range)
  {
    using Value = decltype (std::cbegin (std::declval<Range>()));

    return TaggedRange<Tag, Range, std::unordered_map<Tag, Value>> (tag, range);
  }
}
