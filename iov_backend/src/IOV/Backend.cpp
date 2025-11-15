// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/IOV_Backend.hpp>

namespace mcs
{
  IOV_Backend::IOV_Backend (std::filesystem::path configuration_file)
    : _backend_state
      { decltype (_backend_state)
        { _backend_ops.iov_backend_init (configuration_file)
        , Deleter {std::addressof (_backend_ops)}
        }
      }
  {}

  auto IOV_Backend::write
    ( iov::WriteParam write_parameter
    ) -> iov::Request<iov::WriteRequestData>
  {
    return _backend_ops.iov_backend_write
      ( _backend_state.get()
      , write_parameter
      );
  }

  auto IOV_Backend::read
    ( iov::ReadParam read_parameter
    ) -> iov::Request<iov::ReadRequestData>
  {
    return _backend_ops.iov_backend_read
      ( _backend_state.get()
      , read_parameter
      );
  }

  auto IOV_Backend::allocate (std::size_t size) -> Allocation
  {
    return Allocation
      { new std::span<std::byte>
          ( _backend_ops.iov_backend_allocate
            ( _backend_state.get()
            , size
            )
          )
      , iov_backend::detail::SpanDeleter {_backend_state.get(), _backend_ops}
      };
  }

  auto IOV_Backend::collection_open
    ( iov::CollectionOpenParam meta_data_collection_open_parameter
    ) -> iov::expected<void, iov::ErrorReason>
  {
    return _backend_ops.iov_backend_collection_open
      ( _backend_state.get()
      , meta_data_collection_open_parameter
      );
  }

  auto IOV_Backend::collection_close
    ( iov::CollectionCloseParam meta_data_collection_close_parameter
    ) -> iov::expected<void, iov::ErrorReason>
  {
    return _backend_ops.iov_backend_collection_close
      ( _backend_state.get()
      , meta_data_collection_close_parameter
      );
  }

  auto IOV_Backend::collection_delete
    ( iov::DeleteParam meta_data_collection_delete_parameter
    ) -> iov::Request<iov::DeleteRequestData>
  {
    return _backend_ops.iov_backend_collection_delete
      ( _backend_state.get()
      , meta_data_collection_delete_parameter
      );
  }
}

namespace mcs
{
  IOV_Backend::Deleter::Deleter (iov::BackendOps const* backend_ops)
    : _backend_ops {backend_ops}
  {}

  auto IOV_Backend::Deleter::operator() (void* backend) const noexcept -> void
  {
    _backend_ops->iov_backend_destroy (backend);
  }
}
