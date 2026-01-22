// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <tuple>
#include <utility>

namespace mcs::share_service::provider
{
  template<is_supported_storage_implementation... StorageImplementations>
    Handler<util::type::List<StorageImplementations...>>::Handler
      ( core::Storages<util::type::List<StorageImplementations...>>& storages
      )
        : _storages {storages}
  {}

  namespace
  {
    template< is_supported_storage_implementation StorageImplementation
            , is_supported_storage_implementation... StorageImplementations
            >
      auto create
        ( core::Storages<util::type::List<StorageImplementations...>>& storages
        , command::create::Parameters<StorageImplementation> parameters
        , core::memory::Size size
        )
    {
      auto const read_write_access {storages.read_write_access()};

      auto const storage_id
        { read_write_access.template create<StorageImplementation>
            ( parameters.create
            )
        };

      auto const segment_id
        { read_write_access
          . template modify<StorageImplementation>
            ( storage_id
            , [&] (auto& storage_implementation)
              {
                return storage_implementation.segment_create
                  ( parameters.segment_create
                  , size
                  );
              }
            )
        };

      return std::make_tuple
        ( storage_id
        , SupportedStorageImplementations::id<StorageImplementation>()
        , segment_id
        );
    }
  }


  template<is_supported_storage_implementation... StorageImplementations>
    auto Handler<util::type::List<StorageImplementations...>>::operator()
      ( command::Create command_create
      ) -> command::Create::Response
  {
    return std::visit
      ( [&]<is_supported_storage_implementation StorageImplementation>
          ( command::create::Parameters<StorageImplementation> parameters
          )
        {
          return create<StorageImplementation>
            ( _storages
            , parameters
            , command_create.size
            );
        }
      , command_create.parameters
    );
  }

  template<is_supported_storage_implementation... StorageImplementations>
    auto Handler<util::type::List<StorageImplementations...>>::operator()
      ( command::Remove command_remove
      ) -> command::Remove::Response
  {
    auto const& chunk {command_remove.chunk};

    return std::visit
      ( [&] (auto const& parameters)
        {
          using Parameters = std::remove_cvref_t<decltype (parameters)>;

          auto const read_write_access {_storages.read_write_access()};

          read_write_access.template modify<typename Parameters::Storage>
            ( chunk.storage_id
            , [&] (auto& storage_implementation)
              {
                return storage_implementation.segment_remove
                  ( parameters.segment_remove
                  , chunk.segment_id
                  );
              }
            );

          read_write_access.remove
            ( chunk.storage_id
            );
        }
      , command_remove.parameters
      );
  }

  template<is_supported_storage_implementation... StorageImplementations>
    template<core::chunk::is_access Access>
      auto Handler<util::type::List<StorageImplementations...>>::operator()
        ( command::Attach<Access> command_attach
        ) -> typename command::Attach<Access>::Response
  {
    auto const& chunk {command_attach.chunk};

    return std::visit
      ( [&] ( auto const& parameters
            ) -> core::chunk::Description<Access, StorageImplementations...>
        {
          using Parameters = std::remove_cvref_t<decltype (parameters)>;

          return _storages.read_access()
            . template invoke<typename Parameters::Storage>
              ( chunk.storage_id
              , [&] (auto const& storage_implementation)
                {
                  return storage_implementation
                    . template chunk_description<Access>
                      ( parameters.chunk_description
                      , chunk.segment_id
                      , core::memory::make_range
                        ( core::memory::make_offset (0)
                        , chunk.size
                        )
                      )
                    ;
                }
              )
            ;
        }
      , command_attach.parameters
      );
  }
}
