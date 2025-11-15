// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>

#define MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION(_type...)   \
  MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_type)

#define MCS_SERIALIZATION_DECLARE_INTRUSIVE_CTOR_AND_SAVE(_type)          \
  MCS_SERIALIZATION_DECLARE_INTRUSIVE_CTOR_AND_SAVE_IMPL(_type)

#include "detail/declare.ipp"
