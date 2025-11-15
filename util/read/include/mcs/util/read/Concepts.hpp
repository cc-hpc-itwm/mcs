// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/util/read/read.hpp>
#include <string>

namespace mcs::util::read
{
  template<typename T>
    concept is_readable =
        requires (char const* input)
        {
          { read<T> (input) } -> std::convertible_to<T>;
        }
    || requires (char const* input, std::size_t size)
        {
          { read<T> (input, size) } -> std::convertible_to<T>;
        }
     || requires (std::string const& input)
        {
          { read<T> (input) } -> std::convertible_to<T>;
        }
     ;
}
