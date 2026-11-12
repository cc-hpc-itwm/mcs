// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util::arguments
{
  constexpr View::View
    ( char const* const* arguments
    , std::size_t count
    ) noexcept
      : Base {std::span<char const* const> {arguments, count}}
  {}

  constexpr auto View::from (std::size_t offset) const -> View
  {
    auto const remaining {slice (offset)};

    return View {remaining.data(), remaining.size()};
  }
}
