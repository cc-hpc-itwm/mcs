// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstddef>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <vector>

namespace mcs::core::storage
{
  struct Parameter
  {
    template<serialization::is_serializable T>
      friend auto make_parameter (T const&) -> Parameter;

    template<serialization::is_serializable T> auto as() const -> T;

#ifdef __cpp_lib_constexpr_vector
    constexpr
#endif
              auto operator<=> (Parameter const&) const noexcept = default;

  private:
    struct Make{};

    template<serialization::is_serializable T>
      explicit Parameter (Make, T const&);

    std::vector<std::byte> _blob;

    MCS_UTIL_TUPLISH_ACCESS();

    explicit Parameter (std::vector<std::byte>);
  };
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::core::storage::Parameter);

#include "detail/Parameter.ipp"
