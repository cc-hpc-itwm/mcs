/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_SEGMENT_ID_H
#define MCS_CORE_STORAGE_C_API_SEGMENT_ID_H

#include "types.h"

/* Identifier for a segment in an instance. Segments can be created
   and removed and chunks are parts of segments. Segments are
   conceptually contiguous (bounded) pieces of memory.
*/
struct mcs_core_storage_segment_id
{
  MCS_CORE_STORAGE_SEGMENT_ID value;
};

#endif
