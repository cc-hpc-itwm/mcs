// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <filesystem>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/c_api/Traits.hpp>
#include <mcs/core/storage/c_api/instance.h>
#include <mcs/core/storage/c_api/storage.h>
#include <mcs/core/storage/c_api/types.h>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <memory>
#include <vector>

namespace mcs::core::storage::implementation
{
  // A storage implementation that is based on a c implementation. The
  // Import_C_API implementation redirects requests to the c
  // implementation. All interface calls contain a StorageParameter
  // that is transferred unmodified into the c implementation. It is
  // the responsibility of the users to make sure that the parameters
  // fit with the expectations of the c implementation. All errors
  // from the c implementation are wrapped and annotated by the
  // parameters and then propagated up.
  //
  struct Import_C_API
  {
    using StorageParameter = std::vector<::MCS_CORE_STORAGE_BYTE>;

    struct Tag{};

    struct Parameter
    {
      struct Create
      {
        ::mcs_core_storage _storage;
        StorageParameter _parameter_create;
      };

      struct Size
      {
        struct Max
        {
          StorageParameter _parameter_size_max;
        };

        struct Used
        {
          StorageParameter _parameter_size_used;
        };
      };

      struct Segment
      {
        struct Create
        {
          StorageParameter _parameter_segment_create;
        };

        struct Remove
        {
          StorageParameter _parameter_segment_remove;
        };
      };

      struct Chunk
      {
        struct Description
        {
          StorageParameter _parameter_chunk_description;
        };
      };

      struct File
      {
        struct Read
        {
          StorageParameter _parameter_file_read;
        };

        struct Write
        {
          StorageParameter _parameter_file_write;
        };
      };
    };

    struct Error
    {
      // The implementation does not provide a particular method.
      //
      struct MethodNotProvided : public mcs::Error
      {
        MCS_ERROR_COPY_MOVE_DEFAULT (MethodNotProvided);

      private:
        friend struct Import_C_API;
        MethodNotProvided();
      };

      // The implementation reported an error using the error channel.
      //
      struct Implementation : public mcs::Error
      {
        using Bytes = std::vector<::MCS_CORE_STORAGE_BYTE>;

        [[nodiscard]] constexpr auto error() const noexcept -> Bytes const&;

        MCS_ERROR_COPY_MOVE_DEFAULT (Implementation);

      private:
        friend struct Import_C_API;
        Implementation (Bytes);
        Bytes _error;
      };

      struct Create : public mcs::Error
      {
        [[nodiscard]] constexpr auto parameter
          (
          ) const noexcept -> Parameter::Create const&
          ;

        MCS_ERROR_COPY_MOVE_DEFAULT (Create);

      private:
        friend struct Import_C_API;
        Create (Parameter::Create);
        Parameter::Create _parameter;
      };

      struct Size
      {
        struct Max : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::Size::Max const&
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Max);

        private:
          friend struct Import_C_API;
          Max (Parameter::Size::Max);
          Parameter::Size::Max _parameter;
        };

        struct Used : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::Size::Used const&
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Used);

        private:
          friend struct Import_C_API;
          Used (Parameter::Size::Used);
          Parameter::Size::Used _parameter;
        };
      };

      struct Segment
      {
        struct Create : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::Segment::Create const&
            ;
          [[nodiscard]] constexpr auto size() const noexcept -> memory::Size;

          MCS_ERROR_COPY_MOVE_DEFAULT (Create);

        private:
          friend struct Import_C_API;
          Create (Parameter::Segment::Create, memory::Size);
          Parameter::Segment::Create _parameter;
          memory::Size _size;
        };

        struct Remove : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::Segment::Remove const&
            ;
          [[nodiscard]] constexpr auto segment_id
            (
            ) const noexcept -> segment::ID
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Remove);

        private:
          friend struct Import_C_API;
          Remove (Parameter::Segment::Remove, segment::ID);
          Parameter::Segment::Remove _parameter;
          segment::ID _segment_id;
        };
      };

      struct File
      {
        struct Read : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::File::Read const&
            ;
          [[nodiscard]] constexpr auto segment_id
            (
            ) const noexcept -> segment::ID
            ;
          [[nodiscard]] constexpr auto offset
            (
            ) const noexcept -> memory::Offset
            ;
          [[nodiscard]] constexpr auto path
            (
            ) const noexcept -> std::filesystem::path const&
            ;
          [[nodiscard]] constexpr auto range
            (
            ) const noexcept -> memory::Range
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Read);

        private:
          friend struct Import_C_API;
          Read ( Parameter::File::Read
               , segment::ID
               , memory::Offset
               , std::filesystem::path
               , memory::Range
               );
          Parameter::File::Read _parameter;
          segment::ID _segment_id;
          memory::Offset _offset;
          std::filesystem::path _path;
          memory::Range _range;
        };

        struct Write : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::File::Write const&
            ;
          [[nodiscard]] constexpr auto segment_id
            (
            ) const noexcept -> segment::ID
            ;
          [[nodiscard]] constexpr auto offset
            (
            ) const noexcept -> memory::Offset
            ;
          [[nodiscard]] constexpr auto path
            (
            ) const noexcept -> std::filesystem::path const&
            ;
          [[nodiscard]] constexpr auto range
            (
            ) const noexcept -> memory::Range
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Write);

        private:
          friend struct Import_C_API;
          Write ( Parameter::File::Write
                , segment::ID
                , memory::Offset
                , std::filesystem::path
                , memory::Range
                );
          Parameter::File::Write _parameter;
          segment::ID _segment_id;
          memory::Offset _offset;
          std::filesystem::path _path;
          memory::Range _range;
        };
      };

      struct Chunk
      {
        template<chunk::is_access Access>
          struct Description : public mcs::Error
        {
          [[nodiscard]] constexpr auto parameter
            (
            ) const noexcept -> Parameter::Chunk::Description const&
            ;
          [[nodiscard]] constexpr auto segment_id
            (
            ) const noexcept -> segment::ID
            ;
          [[nodiscard]] constexpr auto range
            (
            ) const noexcept -> memory::Range
            ;

          MCS_ERROR_COPY_MOVE_DEFAULT (Description);

        private:
          friend struct Import_C_API;
          Description ( Parameter::Chunk::Description
                      , segment::ID
                      , memory::Range
                      );
          Parameter::Chunk::Description _parameter;
          segment::ID _segment_id;
          memory::Range _range;
        };
      };

      struct BadAlloc : public mcs::Error
      {
        constexpr auto requested() const noexcept -> memory::Size;
        constexpr auto used() const noexcept -> memory::Size;
        constexpr auto max() const noexcept -> MaxSize;

        MCS_ERROR_COPY_MOVE_DEFAULT (BadAlloc);

      private:
        friend struct Import_C_API;

        BadAlloc
          ( memory::Size requested
          , memory::Size used
          , MaxSize max
          ) noexcept;

        memory::Size _requested;
        memory::Size _used;
        MaxSize _max;
      };
    };

    struct Chunk
    {
      template<chunk::is_access Access>
        struct Description
      {
        struct State
        {
          explicit State (Description<Access> const&);

          auto bytes() const -> typename Access::template Span<std::byte>;

        private:
          using ChunkState = typename c_api::Traits<Access>::ChunkState;
          struct Deleter
          {
            auto operator() (ChunkState*) const -> void;
            util::not_null<::mcs_core_storage const> _storage;
            util::not_null<::mcs_core_storage_instance const> _instance;
          };
          std::unique_ptr<ChunkState, Deleter> _chunk_state;
          typename Access::template Span<std::byte> _bytes
            { util::cast<typename Access::template Pointer<std::byte>>
                ( _chunk_state->data
                )
            , _chunk_state->size
            };
        };

        Description
          ( std::vector<::MCS_CORE_STORAGE_BYTE>
          , util::not_null<::mcs_core_storage const>
          , util::not_null<::mcs_core_storage_instance const>
          ) noexcept
          ;

      private:
        std::vector<::MCS_CORE_STORAGE_BYTE> _description;
        util::not_null<::mcs_core_storage const> _storage;
        util::not_null<::mcs_core_storage_instance const> _instance;

        MCS_SERIALIZATION_ACCESS();
      };
    };

    explicit Import_C_API (Parameter::Create);

    auto size_max
      ( Parameter::Size::Max
      ) const -> MaxSize
      ;
    auto size_used
      ( Parameter::Size::Used
      ) const -> memory::Size
      ;

    auto segment_create
      ( Parameter::Segment::Create
      , memory::Size
      ) -> segment::ID
      ;
    auto segment_remove
      ( Parameter::Segment::Remove
      , segment::ID
      ) -> memory::Size
      ;

    template<chunk::is_access Access>
      auto chunk_description
        ( Parameter::Chunk::Description
        , segment::ID
        , memory::Range
        ) const -> Chunk::Description<Access>
      ;

    auto file_read
      ( Parameter::File::Read
      , segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;
    auto file_write
      ( Parameter::File::Write
      , segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;

  private:
    ::mcs_core_storage _storage;
    struct InstanceDeleter
    {
      auto operator()
        ( ::mcs_core_storage_instance*
        ) const noexcept -> void
        ;

      util::not_null<::mcs_core_storage> _storage;
    };
    std::unique_ptr<::mcs_core_storage_instance, InstanceDeleter> _instance;

    // Call the function with the arguments and an error channel
    // added. If the call produces data on the error channel, then an
    // exception is thrown. If the call does not produce data on the
    // error channel, then the result is returned.
    //
    template<typename Fun, typename... Args>
      static auto invoke_and_throw_on_error (Fun&&, Args&&...);
  };
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::Import_C_API::Chunk::Description<Access>
      );
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access>
      );
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Tag
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Write
  );

#include "detail/Import_C_API.ipp"
