/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_CHANNEL_H
#define MCS_CORE_STORAGE_C_API_CHANNEL_H

#include "types.h"

/* A channel that allows the storage to produce data that is owned by
   the client. A channel is a append-only data structure with dynamic
   size managed by the client. It is used to communicate serialized
   data or error reasons from the storage to the client.
*/
struct mcs_core_storage_channel
{
  /* The channel's sink. All callbacks must use this sink. Not using
     this sink in one of the callbacks below is undefined behavior.
  */
  void* sink;

  /* Append a single byte to a sink. The size grows on-demand.
  */
  void (*push_back)
    ( void* sink
    , MCS_CORE_STORAGE_BYTE
    );

  /* Append multiple bytes. The size grows on-demand.

     Pre: The memory starting at data is at most size bytes in size.
  */
  void (*append)
    ( void* sink
    , MCS_CORE_STORAGE_BYTE const* data
    , MCS_CORE_STORAGE_SIZE size
    );

  /* Reserve space in the sink. No data is appended but the sink
     prepares itself for size many bytes to be appended.
  */
  void (*reserve)
    ( void* sink
    , MCS_CORE_STORAGE_SIZE size
    );
};

#endif
