/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

/* The "chunky" storage is a simple test storage that

   - Does no segment management, all segments are the same.

   - Allocates and de-allocates memory for individual chunks, that
     implies that data does not exists longer than a chunk. This
     behavior is in contrast to a typical storage behavior where data
     life time is related with segment life time and independent from
     chunk life time.

   - Supports const access only, there is no way to modify data in the
     storage. When a chunk is created, then its content is filled with
     a constant value that is part of the parameter for the creation.

   - Respects the maximum size given during construction: A new chunk
     can only be created if it does not exceed the maximum size.

   The chunky storage is not meant to be used in applications but
   is made to test the Virtual storage. For the purpose:

   - It will fail construction if the construction parameter has not
     the correct size. The error will be a copy of the construction
     parameter.

   - It will feed the bad_alloc_channel in segment_create if the
     parameter is not empty.
*/

#ifndef MCS_CORE_TEST_STORAGE_IMPLEMENTATION_CHUNKY_H
#define MCS_CORE_TEST_STORAGE_IMPLEMENTATION_CHUNKY_H

#include <mcs/core/storage/c_api.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void mcs_core_storage_virtual_chunky_error
  ( struct mcs_core_storage_channel* error_channel
  , char const* error
  )
{
  error_channel->append
    ( error_channel->sink
    , (MCS_CORE_STORAGE_BYTE const*) (error)
    , strlen (error)
    );
}

struct chunky_state
{
  MCS_CORE_STORAGE_SIZE size_max;
  MCS_CORE_STORAGE_SIZE size_used;
};

static struct mcs_core_storage_instance
  mcs_core_test_storage_implementation_chunky_construct
    ( struct mcs_core_storage_parameter parameter_create
    , struct mcs_core_storage_channel error_channel
    )
{
  struct mcs_core_storage_instance instance;
  instance.state = (void *) 0;

  MCS_CORE_STORAGE_SIZE size_max;

  if (parameter_create.size != sizeof (size_max))
  {
    mcs_core_storage_virtual_chunky_error
      (&error_channel, "chunky::construct: parameter has the wrong size\0");

    return instance;
  }

  memcpy (&size_max, parameter_create.data, sizeof (size_max));

  instance.state = malloc (sizeof (struct chunky_state));

  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      (&error_channel, "chunky::construct: Could not allocate memory\0");

    return instance;
  }

  struct chunky_state* chunky_state = instance.state;
  chunky_state->size_max = size_max;
  chunky_state->size_used = (MCS_CORE_STORAGE_SIZE) 0;

  return instance;
}

static void
  mcs_core_test_storage_implementation_chunky_destruct
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_channel error_channel
    )
{
  (void) error_channel;

  free (instance.state);
  instance.state = (void *) 0;

  return;
}

static struct mcs_core_storage_memory_size
  mcs_core_test_storage_implementation_chunky_size_max
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_parameter parameter_size_max
    , struct mcs_core_storage_channel error_channel
    )
{
  (void) parameter_size_max;

  struct mcs_core_storage_memory_size size_max;
  size_max.value = (MCS_CORE_STORAGE_SIZE) 0;

  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::size_max: instance.state must not be null\0"
      );

    return size_max;
  }

  struct chunky_state* chunky_state = instance.state;

  size_max.value = chunky_state->size_max;

  return size_max;
}

static struct mcs_core_storage_memory_size
  mcs_core_test_storage_implementation_chunky_size_used
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_parameter parameter_size_used
    , struct mcs_core_storage_channel error_channel
    )
{
  (void) parameter_size_used;

  struct mcs_core_storage_memory_size size_used;
  size_used.value = (MCS_CORE_STORAGE_SIZE) 0;

  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::size_used: instance.state must not be null\0"
      );

    return size_used;
  }

  struct chunky_state* chunky_state = instance.state;

  size_used.value = chunky_state->size_used;

  return size_used;
}

struct mcs_core_storage_segment_id
  mcs_core_test_storage_implementation_chunky_segment_create
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_parameter parameter_segment_create
    , struct mcs_core_storage_memory_size size
    , struct mcs_core_storage_channel bad_alloc_channel
    , struct mcs_core_storage_channel error_channel
    )
{
  struct mcs_core_storage_segment_id segment_id;
  segment_id.value = (MCS_CORE_STORAGE_SEGMENT_ID) 0;

  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::segment_create: instance.state must not be null\0"
      );

    return segment_id;
  }

  struct chunky_state* chunky_state = instance.state;

  if (parameter_segment_create.size > (MCS_CORE_STORAGE_SIZE) 0)
  {
    bad_alloc_channel.reserve
      ( bad_alloc_channel.sink
      , 3 * sizeof (MCS_CORE_STORAGE_SIZE)
      );

    bad_alloc_channel.append
      ( bad_alloc_channel.sink
      , (MCS_CORE_STORAGE_BYTE const*) (&size)
      , sizeof (size)
      );
    bad_alloc_channel.append
      ( bad_alloc_channel.sink
      , (MCS_CORE_STORAGE_BYTE const*) (&chunky_state->size_used)
      , sizeof (chunky_state->size_used)
      );
    bad_alloc_channel.append
      ( bad_alloc_channel.sink
      , (MCS_CORE_STORAGE_BYTE const*) (&chunky_state->size_max)
      , sizeof (chunky_state->size_max)
      );

    return segment_id;
  }

  return segment_id;
}

/* A chunk description is the size and of the chunk and the value to
   be written into the memory. No segment or range information is
   stored.
*/
void
  mcs_core_test_storage_implementation_chunky_chunk_const_description
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_parameter parameter_chunk_const_description
    , struct mcs_core_storage_segment_id segment_id
    , struct mcs_core_storage_memory_range range
    , struct mcs_core_storage_channel description_channel
    , struct mcs_core_storage_channel error_channel
    )
{
  (void) instance;
  (void) segment_id;

  if (parameter_chunk_const_description.size != sizeof (MCS_CORE_STORAGE_BYTE))
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_description: wrong parameter size\0"
      );

    return;
  }

  MCS_CORE_STORAGE_SIZE const size = range.size.value;

  description_channel.reserve
    ( description_channel.sink
    , sizeof (size) + sizeof (MCS_CORE_STORAGE_BYTE)
    );

  description_channel.append
    ( description_channel.sink
    , (MCS_CORE_STORAGE_BYTE const*) (&size)
    , sizeof (size)
    );

  description_channel.append
    ( description_channel.sink
    , parameter_chunk_const_description.data
    , parameter_chunk_const_description.size
    );

  return;
}

/* To create a chunk state means to allocate the memory for the chunk
   and to fill it with the initial value taken from the description.
*/
struct mcs_core_storage_chunk_const_state
  mcs_core_test_storage_implementation_chunky_chunk_const_state
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_parameter chunk_const_description
    , struct mcs_core_storage_channel error_channel
    )
{
  struct mcs_core_storage_chunk_const_state chunk_const_state;
  chunk_const_state.state = (void *) 0;
  chunk_const_state.data = (void *) 0;
  chunk_const_state.size = (MCS_CORE_STORAGE_SIZE) 0;

  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state: instance.state must not be null\0"
      );

    return chunk_const_state;
  }

  struct chunky_state* chunky_state = instance.state;

  if (!chunk_const_description.data)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state: chunk_description.data must not be null\0"
      );

    return chunk_const_state;
  }

  MCS_CORE_STORAGE_SIZE size;
  MCS_CORE_STORAGE_BYTE value;

  if (chunk_const_description.size != sizeof (size) + sizeof (value))
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state: wrong chunk_description.size\0"
      );

    return chunk_const_state;
  }

  memcpy (&size, chunk_const_description.data, sizeof (size));
  memcpy (&value, chunk_const_description.data + sizeof (size), sizeof (value));

  if (chunky_state->size_max > 0)
  {
    if (  (size > chunky_state->size_max)
       || (chunky_state->size_used > chunky_state->size_max - size)
       )
    {
      mcs_core_storage_virtual_chunky_error
        ( &error_channel
        , "chunky::chunk_const_state: not enough memory available in storage\0"
        );

      return chunk_const_state;
    }
  }

  MCS_CORE_STORAGE_BYTE* const data = malloc (size);

  if (!data)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state: could not allocate memory\0"
      );

    return chunk_const_state;
  }

  for ( MCS_CORE_STORAGE_SIZE i = (MCS_CORE_STORAGE_SIZE) 0
      ; i != size
      ; ++i
      )
  {
    data[i] = value;
  }

  chunk_const_state.data = data;
  chunk_const_state.size = size;

  chunky_state->size_used += size;

  return chunk_const_state;
}

void
  mcs_core_test_storage_implementation_chunky_chunk_const_state_destruct
    ( struct mcs_core_storage_instance instance
    , struct mcs_core_storage_chunk_const_state chunk_const_state
    , struct mcs_core_storage_channel error_channel
    )
{
  if (!instance.state)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state_destruct: instance.state must not be null\0"
      );

    return;
  }

  struct chunky_state* chunky_state = instance.state;

  if (chunk_const_state.size > chunky_state->size_used)
  {
    mcs_core_storage_virtual_chunky_error
      ( &error_channel
      , "chunky::chunk_const_state_destruct: size larger than size_used\0"
      );

    return;
  }

  free ((MCS_CORE_STORAGE_BYTE*) (chunk_const_state.data));

  chunky_state->size_used -= chunk_const_state.size;
}

struct mcs_core_storage mcs_core_storage_methods()
{
  struct mcs_core_storage implementation;

  implementation.construct
    = &mcs_core_test_storage_implementation_chunky_construct
    ;
  implementation.destruct
    = &mcs_core_test_storage_implementation_chunky_destruct
    ;
  implementation.size_max
    = &mcs_core_test_storage_implementation_chunky_size_max
    ;
  implementation.size_used
    = &mcs_core_test_storage_implementation_chunky_size_used
    ;
  implementation.segment_create
    = &mcs_core_test_storage_implementation_chunky_segment_create
    ;
  implementation.segment_remove
    = (void*) 0
    ;
  implementation.chunk_const_description
    = &mcs_core_test_storage_implementation_chunky_chunk_const_description
    ;
  implementation.chunk_mutable_description
    = (void*) 0
    ;
  implementation.chunk_const_state
    = &mcs_core_test_storage_implementation_chunky_chunk_const_state
    ;
  implementation.chunk_mutable_state
    = (void*) 0
    ;
  implementation.chunk_const_state_destruct
    = &mcs_core_test_storage_implementation_chunky_chunk_const_state_destruct
    ;
  implementation.chunk_mutable_state_destruct
    = (void*) 0
    ;
  implementation.file_read
    = (void*) 0
    ;
  implementation.file_write
    = (void*) 0
    ;

  return implementation;
}

#endif
