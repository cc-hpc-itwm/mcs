// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  // Tags a range with [tag++, tag++, tag++, ...).
  //
  // Non-owning, the liftime of the range and its elements must be
  // longer than the lifetime of the TaggedRange.
  //
  template<typename Tag, typename Range, typename Container>
    struct TaggedRange
  {
    constexpr TaggedRange (Tag, Range const&);

    // Returns: The range of all tags.
    //
    constexpr auto tags() const noexcept;

    // Returns: The element with the given tag.
    //
    constexpr auto at (Tag const&) const;

  private:
    Container _tagged;
  };

  // tags() will return tags ordered as defined by Tag::operator<
  //
  template<typename Tag, typename Range>
    auto make_ordered_tagged_range (Tag, Range const&);

  // tags() will return tags in unspecified order
  //
  template<typename Tag, typename Range>
    auto make_unordered_tagged_range (Tag, Range const&);
}

#include "detail/TaggedRange.ipp"
