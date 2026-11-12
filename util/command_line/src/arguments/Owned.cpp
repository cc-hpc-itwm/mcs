// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <cstring>
#include <iterator>
#include <mcs/util/arguments/Owned.hpp>
#include <stack>
#include <string_view>
#include <vector>

namespace mcs::util::arguments
{
  Owned::Owned (View arguments)
    : Base { std::vector<char const*>
             { std::begin (arguments), std::end (arguments)
             }
           }
  {}

  auto Owned::operator+= (util::not_null<char const> argument) -> Owned&
  {
    _storage.emplace_back (argument.get());

    return *this;
  }

  auto Owned::operator-= (util::not_null<char const> argument) -> Owned&
  {
    auto embeddings {std::stack<std::string_view>{}};
    std::erase_if
      ( _storage
      , [&] (char const* candidate)
        {
          return detail::is_top_level_argument (embeddings, candidate)
            && std::strcmp (candidate, argument.get()) == 0
            ;
        }
      );

    return *this;
  }

  auto Owned::view() const noexcept -> View
  {
    return View {_storage.data(), size()};
  }

  auto Owned::from (std::size_t offset) const -> View
  {
    auto const remaining {slice (offset)};

    return View {remaining.data(), remaining.size()};
  }
}
