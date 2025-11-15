// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define fmt, read and serialization for tuplish classes. The different
// versions accept different number of members.
//
// EXAMPLE
//
// S.hpp:
// namespace ns { struct S { T a; U b; }; }
//
// MCS_UTIL_TUPLISH_DEFINE_FMT_READ2 ("ns::S", ns::S, a, b)
//
// S.cpp
// MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2 (ns::S, a, b)
//
// -> will print and parse "ns::S (a, b)"
//
#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ0(_prefix, _type)                           \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ0_IMPL(_prefix, _type)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ1(_prefix, _type, _m0)                      \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_IMPL(_prefix, _type, _m0)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ2(_prefix, _type, _m0, _m1)                 \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ2_IMPL(_prefix, _type, _m0, _m1)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ3(_prefix, _type, _m0, _m1, _m2)            \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ3_IMPL(_prefix, _type, _m0, _m1, _m2)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ4(_prefix, _type, _m0, _m1, _m2, _m3)       \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ4_IMPL(_prefix, _type, _m0, _m1, _m2, _m3)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ5(_prefix, _type, _m0, _m1, _m2, _m3, _m4)  \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ5_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ6(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ6_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ7(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ7_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ8(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ8_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ9(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ9_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ10(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ10_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ11(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ11_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10)

#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ12(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11) \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ12_IMPL(_prefix, _type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1(_type, _m0)                          \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1_IMPL(_type, _m0)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2(_type, _m0, _m1)                     \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2_IMPL(_type, _m0, _m1)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3(_type, _m0, _m1, _m2)                \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3_IMPL(_type, _m0, _m1, _m2)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION4(_type, _m0, _m1, _m2, _m3)           \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION4_IMPL(_type, _m0, _m1, _m2, _m3)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION5(_type, _m0, _m1, _m2, _m3, _m4)      \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION5_IMPL(_type, _m0, _m1, _m2, _m3, _m4)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION6(_type, _m0, _m1, _m2, _m3, _m4, _m5)  \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION6_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION7(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION7_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION8(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION8_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION9(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION9_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION10(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION10_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION11(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION11_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10)

#define MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION12(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11) \
  MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION12_IMPL(_type, _m0, _m1, _m2, _m3, _m4, _m5, _m6, _m7, _m8, _m9, _m10, _m11)

// Avoid make_tuple around a single member
//
// EXAMPLE
//
// S.hpp:
// namespace ns { struct S { T x;}; }
//
// MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE ("ns::S", ns::S, x)
//
// -> will print and parse "ns::S x"
//
#define MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE(_prefix, _type, _m0)   \
  MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE_IMPL(_prefix, _type, _m0)

#include "detail/define.ipp"
