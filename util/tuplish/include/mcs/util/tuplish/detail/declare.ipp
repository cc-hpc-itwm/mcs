// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <mcs/util/require_semi.hpp>

#define MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION_IMPL(_type)                 \
  namespace fmt                                                            \
  {                                                                        \
    template<> MCS_UTIL_FMT_DECLARE (_type);                               \
  }                                                                        \
                                                                           \
  namespace mcs::serialization                                             \
  {                                                                        \
    template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (_type);  \
  }                                                                        \
                                                                           \
  namespace mcs::util::read                                                \
  {                                                                        \
    template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (_type);  \
  } MCS_UTIL_REQUIRE_SEMI()
