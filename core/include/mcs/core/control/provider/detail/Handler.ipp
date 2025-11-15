// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::core::control::provider
{
  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Handler
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      ) noexcept
        : _storages {storages}
  {}

  template<storage::is_implementation... StorageImplementations>
    template<chunk::is_access Access>
      auto Handler<StorageImplementations...>::operator()
        ( command::chunk::Description<Access, StorageImplementations...>
            chunk_description
        ) const -> typename command::chunk::
                     Description<Access, StorageImplementations...>::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , chunk_description.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          ) -> chunk::Description<Access, StorageImplementations...>
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.template chunk_description<Access>
            ( chunk_description.storage_parameter
              .template as<typename Parameter::Chunk::Description>()
            , chunk_description.segment_id
            , chunk_description.memory_range
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::file::Read file_read
      ) const -> command::file::Read::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , file_read._storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.file_read
            ( file_read._parameter_file_read
              .template as<typename Parameter::File::Read>()
            , file_read._segment_id
            , file_read._offset
            , file_read._file
            , file_read._range
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::file::Write file_write
      ) const -> command::file::Write::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , file_write._storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.file_write
            ( file_write._parameter_file_write
              .template as<typename Parameter::File::Write>()
            , file_write._segment_id
            , file_write._offset
            , file_write._file
            , file_write._range
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::segment::Create segment_create
      ) -> command::segment::Create::Response
  {
    return _storages->visit
      ( _storages->write_access()
      , segment_create.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.segment_create
            ( segment_create.storage_parameter
              .template as<typename Parameter::Segment::Create>()
            , segment_create.memory_size
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::segment::Remove segment_remove
      ) -> command::segment::Remove::Response
  {
    return _storages->visit
      ( _storages->write_access()
      , segment_remove.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.segment_remove
            ( segment_remove.storage_parameter
              .template as<typename Parameter::Segment::Remove>()
            , segment_remove.segment_id
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::storage::Create<StorageImplementations...> storage_create
      ) -> typename command::storage::Create<StorageImplementations...>::Response
  {
    return storage_create.implementation_id.run
      ( [&]<storage::is_implementation StorageImplementation>()
        {
          return _storages->template create<StorageImplementation>
            ( _storages->write_access()
            , storage_create
              . storage_parameter
              . template as<typename StorageImplementation::Parameter::Create>()
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::storage::Remove storage_remove
      ) -> command::storage::Remove::Response
  {
    return _storages->remove
      ( _storages->write_access()
      , storage_remove.storage_id
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::storage::Size size
      ) -> command::storage::Size::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , size.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return command::storage::Size::Response
            { implementation.size_max
              ( size.storage_parameter_size_max
                .template as<typename Parameter::Size::Max>()
              )
            , implementation.size_used
              ( size.storage_parameter_size_used
                .template as<typename Parameter::Size::Used>()
              )
            };
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::storage::size::Max size_max
      ) const -> command::storage::size::Max::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , size_max.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.size_max
            ( size_max.storage_parameter
              .template as<typename Parameter::Size::Max>()
            );
        }
      );
  }

  template<storage::is_implementation... StorageImplementations>
    auto Handler<StorageImplementations...>::operator()
      ( command::storage::size::Used size_used
      ) const -> command::storage::size::Used::Response
  {
    return _storages->visit
      ( _storages->read_access()
      , size_used.storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return implementation.size_used
            ( size_used.storage_parameter
              .template as<typename Parameter::Size::Used>()
            );
        }
      );
  }
}
