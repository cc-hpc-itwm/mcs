// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/parse.hpp>
#include <mcs/util/read_file.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::util::read
{
  template<typename T, typename Char>
    constexpr auto read (State<Char>& state) -> T
  {
    auto value {parse<T> (state)};
    if (!state.end())
    {
      throw state.error ("read: Additional input");
    }
    return value;
  }

  template<typename T, typename... Args>
    [[nodiscard]] constexpr auto read (Args&&... args) -> T
  {
    auto state {make_state (std::forward<Args> (args)...)};
    return read<T> (state);
  }

  template<typename T>
    [[nodiscard]] auto from_file (std::filesystem::path path) -> T
  {
    auto const content {read_file (path)};
    return read<T> (content);
  }
}
