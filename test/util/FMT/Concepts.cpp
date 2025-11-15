// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdio>
#include <fmt/ostream.h>
#include <gtest/gtest.h>
#include <mcs/util/FMT/Concepts.hpp>
#include <ostream>

namespace mcs::util::FMT
{
  TEST (UtilFMTConcepts, file_pointer_is_sink)
  {
    static_assert (sink<FILE*>);
  }
  TEST (UtilFMTConcepts, ostream_reference_is_sink)
  {
    static_assert (sink<std::ostream&>);
  }
}
