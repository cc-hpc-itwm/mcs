// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/cast.hpp>

namespace mcs::core
{
  template< chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    Chunk<Access, StorageImplementations...>::Chunk
      ( chunk::Description<Access, StorageImplementations...> chunk_description
      )
      : _state
        { std::visit
          ( []<typename Description>
               ( Description const& description
               )
            {
              return decltype (_state)
                { std::in_place_type<typename Description::State>
                , description
                };
            }
          , chunk_description
          )
        }
  {}
}

namespace mcs::core
{
  template< chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    auto Chunk<Access, StorageImplementations...>::data
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return std::visit
      ( [] (auto const& state) noexcept
        {
          return state.bytes();
        }
      , _state
      );
  }
}

namespace mcs::core
{
  template< chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    [[nodiscard]] auto make_chunk
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      , storage::ID storage_id
      , storage::Parameter parameter_chunk_description
      , storage::segment::ID segment_id
      , memory::Range memory_range
      ) -> Chunk<Access, StorageImplementations...>
  {
    return storages->visit
      ( storages->read_access()
      , storage_id
      , [&]<storage::is_implementation StorageImplementation>
          ( StorageImplementation const& implementation
          )
        {
          using Parameter = StorageImplementation::Parameter;

          return Chunk<Access, StorageImplementations...>
            { implementation.template chunk_description<Access>
              ( parameter_chunk_description
                .as<typename Parameter::Chunk::Description>()
              , segment_id
              , memory_range
              )
            };
        }
      );
  }
}

namespace mcs::core
{
  template< typename T
          , chunk::is_access Access
          , storage::is_implementation... StorageImplementations
          >
    auto as
      ( Chunk<Access, StorageImplementations...> const& chunk
      ) -> typename Access::template Span<T>
  {
    auto bytes {chunk.data()};

    return {util::cast<T*> (bytes.data()), bytes.size() / sizeof (T)};
  }
}
