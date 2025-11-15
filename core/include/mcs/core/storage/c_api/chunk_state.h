/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_CHUNK_STATE_H
#define MCS_CORE_STORAGE_C_API_CHUNK_STATE_H

#include "types.h"

/* The chunk_{const,mutable} state is the span of memory to be used by
   the client. The memory is owned by the storage. Clients will end
   the life time by calls to chunk_{const,mutable}_state_destruct. The
   field state allows storages to store internal information. The
   client will not look into the field state in any way, ever.
*/
struct mcs_core_storage_chunk_const_state
{
  void* state;
  MCS_CORE_STORAGE_BYTE const* data;
  MCS_CORE_STORAGE_SIZE size;
};
struct mcs_core_storage_chunk_mutable_state
{
  void* state;
  MCS_CORE_STORAGE_BYTE* data;
  MCS_CORE_STORAGE_SIZE size;
};

#endif
