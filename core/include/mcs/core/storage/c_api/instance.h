/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_INSTANCE_H
#define MCS_CORE_STORAGE_C_API_INSTANCE_H

/* State for an instance of the storage. The client will not look into
   the instance, ever. The instance is produced and destroyed by the
   storage and handed back to it by the client.
*/
struct mcs_core_storage_instance
{
  void* state;
};

#endif
