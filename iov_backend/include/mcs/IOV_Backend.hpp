// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <filesystem>
#include <iov/iov.hpp>
#include <mcs/IOV_Database.hpp>
#include <mcs/iov_backend/detail/SpanDeleter.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>
#include <span>

namespace mcs
{
  struct IOV_Backend
  {
    IOV_Backend (std::filesystem::path configuration_file);

    [[nodiscard]] auto write
      ( iov::WriteParam write_parameter
      ) -> iov::Request<iov::WriteRequestData>
      ;

    [[nodiscard]] auto read
      ( iov::ReadParam read_parameter
      ) -> iov::Request<iov::ReadRequestData>
      ;

    using Allocation
      = std::unique_ptr<std::span<std::byte>, iov_backend::detail::SpanDeleter>
      ;

    [[nodiscard]] auto allocate (std::size_t) -> Allocation;

    [[nodiscard]] auto collection_open
      ( iov::CollectionOpenParam
      ) -> iov::expected<void, iov::ErrorReason>
      ;
    [[nodiscard]] auto collection_close
      ( iov::CollectionCloseParam
      ) -> iov::expected<void, iov::ErrorReason>
      ;
    [[nodiscard]] auto collection_delete
      ( iov::DeleteParam
      ) -> iov::Request<iov::DeleteRequestData>
      ;

    // \todo remove!?, required because Collection::delete calls back
    // into the context
    auto context (util::not_null<IOV_Database> database) -> iov::BackendContext
    {
      return iov::BackendContext
        { .backend_id = iov::BackendContext::UUID {0}
        , .md_backend
            = iov::meta::MetadataBackend
              { std::unique_ptr<iov::meta::AbstractDatabaseBackend>
                  { new mcs::IOV_DBRef {database}
                  }
              }
        , .backend_implementation = iov::DLHandle {nullptr}
        , .ops = _backend_ops
        , .state = _backend_state.get()
        };
    }

  private:
    iov::BackendOps _backend_ops {iov::iov_backend_ops()};

    struct Deleter
    {
      explicit Deleter (iov::BackendOps const*);
      auto operator() (void* backend) const noexcept -> void;

    private:
      iov::BackendOps const* _backend_ops;
    };

    std::unique_ptr<void, Deleter> _backend_state;
  };
}
