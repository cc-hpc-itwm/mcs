/* Copyright (C) 2025 Fraunhofer ITWM
   License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE
*/

/*
   ABI stable C-interface for a storage implementation.

   The interface consists of a couple of helper structures and
   functions and the storage itself that is defined by callbacks to
   manage segments and chunks.

   Clients of the storage will create instances and hand those
   instances back to the storage when calling methods.

   Error handling is implemented using error channels. Those error
   channels allow the storage to transport error reasons and generally
   a call is considered an error if and only if there is at least one
   byte appended to the error channel. Operations that do not append
   anything to the error channel are considered a success. Data in the
   error channel will be presented to users as reason for failure in
   textual form, i.e. the bytes are interpreted as a string.
*/

#ifndef MCS_CORE_STORAGE_C_API_H
#define MCS_CORE_STORAGE_C_API_H

// NOLINTBEGIN (modernize-use-trailing-return-type), it is C

#include "c_api/storage.h"

#if __cplusplus
extern "C"
#endif

struct mcs_core_storage mcs_core_storage_methods();

// NOLINTEND (modernize-use-trailing-return-type)

#endif
