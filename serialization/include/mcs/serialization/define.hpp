// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

#define MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT(_oa, _name, _type...) \
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT_IMPL(_oa, _name, _type)

#define MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT(_ia, _type...) \
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT_IMPL(_ia, _type)

#define MCS_SERIALIZATION_LOAD_FIELD(_ia, _field, _type...) \
  MCS_SERIALIZATION_LOAD_FIELD_IMPL(_ia, _field, _type)

#define MCS_SERIALIZATION_SAVE_FIELD(_oa, _name, _field)    \
  MCS_SERIALIZATION_SAVE_FIELD_IMPL(_oa, _name, _field)

#include "detail/define.ipp"
