// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdlib>
#include <mcs/fuse/content/implementation/Realloc.hpp>

namespace mcs::fuse::content::implementation
{
  Realloc::~Realloc()
  {
    std::free (_data);
  }
}
