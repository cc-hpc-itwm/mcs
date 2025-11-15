/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_MEMORY_OFFSET_H
#define MCS_CORE_STORAGE_C_API_MEMORY_OFFSET_H

#include "types.h"

/* Offset of a piece of memory, that is the distance from the begin of
   a segment.
*/
struct mcs_core_storage_memory_offset
{
  MCS_CORE_STORAGE_OFFSET value;
};

#endif
