/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

#ifndef MCS_CORE_STORAGE_C_API_STORAGE_H
#define MCS_CORE_STORAGE_C_API_STORAGE_H

#include "channel.h"
#include "chunk_state.h"
#include "instance.h"
#include "memory_offset.h"
#include "memory_range.h"
#include "memory_size.h"
#include "parameter.h"
#include "segment_id.h"

/* The callback functions of a storage. They contain methods to create
   and destruct instances, to manage segments and chunks and to read
   and write files.
*/
struct mcs_core_storage
{
  /* Construct an instance of the storage. The client will not look
     into the instance in any way, ever. The instance is handed back
     to the storage in later calls.

     Instances returned in a successful call to construct will be
     destructed exactly once. Instance returned in a failing call to
     construct are forgotten immediately and not used in any way.
  */
  struct mcs_core_storage_instance (*construct)
    ( struct mcs_core_storage_parameter parameter_create
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Destruct an instance of the storage. The client guarantees that
     the instance was produced by an earlier successful call to
     construct. The client guarantees that every valid instance (that
     is an instance that was returned by a successful call to
     construct) will destructed exactly once.
   */
  void (*destruct)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Query the maximum size of an instance of the storage.

     The value 0 means "Unlimited".
  */
  struct mcs_core_storage_memory_size (*size_max)
   ( struct mcs_core_storage_instance
   , struct mcs_core_storage_parameter parameter_size_max
   , struct mcs_core_storage_channel error_channel
   )
   ;

  /* Query the currently used size of an instance of the storage. The
     used size is the sum of the sizes of all segments.
  */
  struct mcs_core_storage_memory_size (*size_used)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_size_used
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Create a segment of a certain size in an instance of the
     storage. A segment is a conceptually contiguous piece of
     memory. If there is a size limit, then the sum of the sizes of
     all segments will not exceed that limit.

     The bad_alloc_channel is a specific error channel that is used by
     the client to create an BadAlloc exception. The storage should,
     in case there is not enough memory left, write exactly three
     values of type MCS_CORE_STORAGE_SIZE into the bad_alloc_channel:
     size_requested, size_used, size_max.

     The error_channel has priority over the bad_alloc_channel: If
     there is data in the error_channel, then the client will produce
     throw an Error::Implementation and not look into the
     bad_alloc_channel.
  */
  struct mcs_core_storage_segment_id (*segment_create)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_segment_create
    , struct mcs_core_storage_memory_size
    , struct mcs_core_storage_channel bad_alloc_channel
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Remove a segment from an instance of the storage.
  */
  struct mcs_core_storage_memory_size (*segment_remove)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_segment_remove
    , struct mcs_core_storage_segment_id
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Retrieve a description of a chunk. A chunk description is a
     serialize-able object and the storage must fill a buffer in the
     client with the serialized data. This serialized data will be
     presented back to the storage in calls to
     chunk_{const,mutable}_state in order to produce chunks. The
     client will not look into the content produced in the description
     channel, ever. The client might copy the data from the
     description channel.

     Pre: The segment exists.
     Pre: The range is within the bounds of the segment.
  */
  void (*chunk_const_description)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_chunk_description
    , struct mcs_core_storage_segment_id
    , struct mcs_core_storage_memory_range
    , struct mcs_core_storage_channel description_channel
    , struct mcs_core_storage_channel error_channel
    )
    ;
  void (*chunk_mutable_description)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_chunk_description
    , struct mcs_core_storage_segment_id
    , struct mcs_core_storage_memory_range
    , struct mcs_core_storage_channel description_channel
    , struct mcs_core_storage_channel error_channel
    )
    ;

  /* Produce and destruct a chunk_{const,mutable}_state from a
     chunk_{const,mutable}_description. The chunk state allows for
     random load/store access into the memory. The client guarantees
     that the chunk description has been produced by a successful call
     to chunk_description and that the segment has not been
     removed. Successfully created chunk states are destructed exactly
     once by a call to chunk_{const,mutable}_destruct.
  */
  struct mcs_core_storage_chunk_const_state (*chunk_const_state)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter chunk_const_description
    , struct mcs_core_storage_channel error_channel
    )
    ;
  struct mcs_core_storage_chunk_mutable_state (*chunk_mutable_state)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter chunk_mutable_description
    , struct mcs_core_storage_channel error_channel
    )
    ;

  void (*chunk_const_state_destruct)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_chunk_const_state
    , struct mcs_core_storage_channel error_channel
    );
  void (*chunk_mutable_state_destruct)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_chunk_mutable_state
    , struct mcs_core_storage_channel error_channel
    );

  /* Transfer data between storage and files.

     file_{read,write} are optional. If the storage does not provide
     them, then the client will use a fallback that copies data
     between chunks and files.
  */
  struct mcs_core_storage_memory_size (*file_read)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_file_read
    , struct mcs_core_storage_segment_id
    , struct mcs_core_storage_memory_offset
    , struct mcs_core_storage_parameter file_name
    , struct mcs_core_storage_memory_range
    , struct mcs_core_storage_channel error_channel
    )
    ;

  struct mcs_core_storage_memory_size (*file_write)
    ( struct mcs_core_storage_instance
    , struct mcs_core_storage_parameter parameter_file_write
    , struct mcs_core_storage_segment_id
    , struct mcs_core_storage_memory_offset
    , struct mcs_core_storage_parameter file_name
    , struct mcs_core_storage_memory_range
    , struct mcs_core_storage_channel error_channel
    )
    ;
};

#endif
