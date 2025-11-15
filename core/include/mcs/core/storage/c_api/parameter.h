/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_PARAMETER_H
#define MCS_CORE_STORAGE_C_API_PARAMETER_H

#include "types.h"

/* A non-owning piece of constant memory. Used to communicate
   (serialized) (type erased) parameters from the client to the
   storage. The serialization method depends on the storage and it is
   the responsibility of the client to use the correct serialization
   method.
*/
struct mcs_core_storage_parameter
{
  MCS_CORE_STORAGE_BYTE const* data;
  MCS_CORE_STORAGE_SIZE size;
};

#endif
