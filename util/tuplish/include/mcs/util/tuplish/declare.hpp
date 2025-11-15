// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Declare fmt, read and serialization for a tuplish class.
//
// EXAMPLE
//
// namespace ns { struct S {...}; }
//
// MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (ns::S);
//
#define MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION(_type)   \
  MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION_IMPL(_type)

#include "detail/declare.ipp"
