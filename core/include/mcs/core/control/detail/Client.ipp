// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <memory>

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    template<typename Executor>
      Client< Protocol
            , AccessPolicy
            , util::type::List<StorageImplementations...>
            >::Client
      ( Executor& io_context
      , util::ASIO::Connectable<Protocol> provider_connectable
      )
        : Base
          { io_context
          , provider_connectable
          , std::make_shared<AccessPolicy>()
          }
  {}
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    template<storage::is_implementation StorageImplementation>
      auto Client< Protocol
                 , AccessPolicy
                 , util::type::List<StorageImplementations...>
                 >::storage_create
        ( typename StorageImplementation::Parameter::Create parameter_create
        ) const -> storage::ID
  {
    return Base::operator()
      ( command::storage::create< StorageImplementation
                                          , StorageImplementations...
                                          >
          ( parameter_create
          )
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::storage_remove
      ( storage::ID storage_id
      ) const -> void
  {
    return Base::operator()
      ( command::storage::Remove {storage_id}
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::storage_size_max
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      ) const -> storage::MaxSize
  {
    return Base::operator()
      ( command::storage::size::Max
        { storage_id
        , storage_parameter
        }
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::storage_size_used
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      ) const -> memory::Size
  {
    return Base::operator()
      ( command::storage::size::Used
        { storage_id
        , storage_parameter
        }
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::storage_size
      ( storage::ID storage_id
      , storage::Parameter storage_parameter_size_max
      , storage::Parameter storage_parameter_size_used
      ) const -> command::storage::Size::Response
  {
    return Base::operator()
      ( command::storage::Size
        { storage_id
        , storage_parameter_size_max
        , storage_parameter_size_used
        }
      );
  }
}
namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::segment_create
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      , memory::Size memory_size
      ) const -> storage::segment::ID
  {
    return Base::operator()
      ( command::segment::Create
        { storage_id
        , storage_parameter
        , memory_size
        }
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::segment_remove
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      , storage::segment::ID segment_id
      ) const -> memory::Size
  {
    return Base::operator()
      ( command::segment::Remove
        { storage_id
        , storage_parameter
        , segment_id
        }
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    template<chunk::is_access Access>
      auto Client< Protocol
                 , AccessPolicy
                 , util::type::List<StorageImplementations...>
                 >::chunk_description
          ( storage::ID storage_id
          , storage::Parameter storage_parameter
          , storage::segment::ID segment_id
          , memory::Range memory_range
          ) const -> chunk::Description<Access, StorageImplementations...>
  {
    return Base::operator()
      ( command::chunk::Description<Access, StorageImplementations...>
        { storage_id
        , storage_parameter
        , segment_id
        , memory_range
        }
      );
  }
}

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::file_read
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      , storage::segment::ID segment_id
      , memory::Offset offset
      , std::filesystem::path path
      , memory::Range range
      ) const -> memory::Size
  {
    return Base::operator()
      ( command::file::Read
        { storage_id
        , storage_parameter
        , segment_id
        , offset
        , path
        , range
        }
      );
  }

  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::file_write
      ( storage::ID storage_id
      , storage::Parameter storage_parameter
      , storage::segment::ID segment_id
      , memory::Offset offset
      , std::filesystem::path path
      , memory::Range range
      ) const -> memory::Size
  {
    return Base::operator()
      ( command::file::Write
        { storage_id
        , storage_parameter
        , segment_id
        , offset
        , path
        , range
        }
      );
  }
}
