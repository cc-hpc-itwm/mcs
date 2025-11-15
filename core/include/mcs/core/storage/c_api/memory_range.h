/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_MEMORY_RANGE_H
#define MCS_CORE_STORAGE_C_API_MEMORY_RANGE_H

#include "memory_offset.h"
#include "memory_size.h"

/* A range of memory within a segment. The range describes the bytes
   in the half open interval [begin, begin + size).
*/
struct mcs_core_storage_memory_range
{
  struct mcs_core_storage_memory_offset begin;
  struct mcs_core_storage_memory_size size;
};

#endif
