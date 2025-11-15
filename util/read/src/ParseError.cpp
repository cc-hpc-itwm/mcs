// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/ParseError.hpp>
#include <utility>

namespace mcs::util::read
{
  ParseError::ParseError (std::string reason)
    : mcs::Error {reason}
  {}
  ParseError::~ParseError() = default;
}
