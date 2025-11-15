// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/c_api/Traits.hpp>
#include <mcs/core/storage/c_api/storage.h>

namespace mcs::core::storage
{
  // Implement the C API using an existing storage implementation.
  //
  // EXAMPLE:
  //
  // Produce a shared object for an existing storage implementation:
  //
  //   extern "C" struct ::mcs_core_storage mcs_core_storage_methods()
  //   {
  //     return std::invoke
  //       ( mcs::core::storage::Implement_C_API::methods<Implementation>
  //       );
  //   }
  //
  struct Implement_C_API
  {
  public:
    template<is_implementation Implementation>
      static auto methods() noexcept -> ::mcs_core_storage;

    struct Error
    {
      struct InstanceMustNotBeNull : mcs::Error
      {
        InstanceMustNotBeNull();
        MCS_ERROR_COPY_MOVE_DEFAULT (InstanceMustNotBeNull);
      };
    };

  private:
    template<is_implementation Implementation>
      static auto construct
        ( ::mcs_core_storage_parameter parameter_create
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_instance
      ;

    template<is_implementation Implementation>
      static auto destruct
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> void
      ;

    template<is_implementation Implementation>
      static auto size_max
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_size_max
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_memory_size
      ;

    template<is_implementation Implementation>
      static auto size_used
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_size_used
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_memory_size
      ;

    template<is_implementation Implementation>
      static auto segment_create
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_segment_create
        , ::mcs_core_storage_memory_size
        , ::mcs_core_storage_channel bad_alloc_channel
        , ::mcs_core_storage_channel error_channel
        ) -> ::mcs_core_storage_segment_id
      ;

    template<is_implementation Implementation>
      static auto segment_remove
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_segment_remove
        , ::mcs_core_storage_segment_id
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_memory_size
      ;

    template<is_implementation Implementation, chunk::is_access Access>
      static auto chunk_description
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_chunk_description
        , ::mcs_core_storage_segment_id
        , ::mcs_core_storage_memory_range
        , ::mcs_core_storage_channel description_channel
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> void
      ;

    template<is_implementation Implementation, chunk::is_access Access>
      static auto chunk_state
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter chunk_description
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> typename c_api::Traits<Access>::ChunkState
      ;

    template<is_implementation Implementation, chunk::is_access Access>
      static auto chunk_state_destruct
        ( ::mcs_core_storage_instance
        , typename c_api::Traits<Access>::ChunkState
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> void
      ;

    template<is_implementation Implementation>
      static auto file_read
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_file_read
        , ::mcs_core_storage_segment_id
        , ::mcs_core_storage_memory_offset
        , ::mcs_core_storage_parameter file_name
        , ::mcs_core_storage_memory_range
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_memory_size
      ;
    template<is_implementation Implementation>
      static auto file_write
        ( ::mcs_core_storage_instance
        , ::mcs_core_storage_parameter parameter_file_write
        , ::mcs_core_storage_segment_id
        , ::mcs_core_storage_memory_offset
        , ::mcs_core_storage_parameter file_name
        , ::mcs_core_storage_memory_range
        , ::mcs_core_storage_channel error_channel
        ) noexcept -> ::mcs_core_storage_memory_size
      ;
  };
}

#include "detail/Implement_C_API.ipp"
