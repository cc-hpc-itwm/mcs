// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/UniqID.hpp>

namespace mcs::testing
{
  auto UniqID::operator()() const -> std::string
  {
    return _id;
  }
}
