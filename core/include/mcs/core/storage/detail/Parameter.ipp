// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/read/STD/vector.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::core::storage
{
  template<serialization::is_serializable T>
    Parameter::Parameter (Make, T const& x)
      : Parameter {serialization::OArchive {x}.bytes()}
  {}

  template<serialization::is_serializable T>
    auto Parameter::as() const -> T
  {
    return serialization::load_from<T> (_blob);
  }

  template<serialization::is_serializable T>
    auto make_parameter (T const& x) -> Parameter
  {
    return Parameter {Parameter::Make{}, x};
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE
  ( "Parameter "
  , mcs::core::storage::Parameter
  , _blob
  );
