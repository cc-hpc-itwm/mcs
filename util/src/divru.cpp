// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/divru.hpp>

namespace mcs::util
{
  Divru::DivisionByZero::DivisionByZero()
    : mcs::Error {"DivisionByZero"}
  {}
  Divru::DivisionByZero::~DivisionByZero() = default;

  Divru::OverflowInIntermediateValue::OverflowInIntermediateValue()
    : mcs::Error {"OverflowInIntermediateValue"}
  {}
  Divru::OverflowInIntermediateValue::~OverflowInIntermediateValue() = default;
}
