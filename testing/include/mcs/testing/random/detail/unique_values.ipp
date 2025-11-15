// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <iterator>
#include <set>

namespace mcs::testing::random
{
  template<std::totally_ordered T, typename Generator, typename Sink>
    requires (is_generator<Generator, T> && is_sink<Sink, T>)
    auto unique_values
      ( Generator& generator
      , Sink& sink
      , std::size_t n
      ) noexcept -> void
  {
    std::set<T> _seen;

    while (_seen.size() < n)
    {
      auto const x {_seen.emplace (generator())};

      if (x.second)
      {
        sink (*x.first);
      }
    }
  }

  template<std::totally_ordered T, typename Generator>
    requires (is_generator<Generator, T>)
    auto unique_values
      ( Generator& generator
      , std::size_t n
      ) noexcept -> std::vector<T>
  {
    std::vector<T> values;
    auto sink
      { [&] (auto value)
        {
          values.emplace_back (value);
        }
      };

    unique_values<T> (generator, sink, n);

    return values;
  }
}
