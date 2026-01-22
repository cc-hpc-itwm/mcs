// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/util/HeterogeneousMap.hpp>

namespace mcs::core
{
  // Heterogeneous container for storages.
  //
  // EXAMPLE:
  //
  //    using SupportedStorageImplementations = util::type::List<A, B, ...>;
  //
  //    auto storages {Storages<SupportedStorageImplementations>{}};
  //
  //    {
  //      // use some help from UniqueStorage
  //      auto const storage
  //        { make_unique_storage<A>
  //          ( std::addressof (storages)
  //          , A::Parameter::Create {...}
  //          )
  //        };
  //
  //      // keep the access token for more than one operation
  //      {
  //        auto const read_access {storages.read_access()};
  //
  //        // typed access providing the (known) type
  //        read_access.template invoke<A>
  //          ( storage->id()
  //          , [&] (auto const& storage_implementation)
  //            {
  //              return storage_implementation.size_max
  //                ( A::Parameter::Size::Max{}
  //                );
  //            }
  //          );
  //
  //        // untyped access
  //        read_access.visit
  //          ( storage->id()
  //          , [&]<storage::is_implementation StorageImplemenation>
  //              ( StorageImplementation const& storage_implementation
  //              )
  //            {
  //              static_assert (std::is_same_v<A, StorageImplementation>);
  //              ...
  //            }
  //          );
  //       } // give up the access token
  //     } // destroy the storage and remove from storages
  //
  // EXAMPLE:
  //
  //    auto storages {Storages<SupportedStorageImplementations>{}};
  //
  //    // create a storage of type B manually, ad-hoc access token
  //    auto const storage_id
  //      { storages.read_write_access().template create<B>
  //        ( B::Parameter::Create {...}
  //        )
  //      };
  //
  //    // create and remove a segment in the storage, re-use access token
  //    {
  //      auto const read_write_access {storages.read_write_access()};
  //
  //      auto const segment_id
  //        { read_write_access.visit
  //            ( storage_id
  //            , [&]<storage::is_implementation StorageImplementation>
  //                ( auto& storage_implementation
  //                )
  //              {
  //                return storage_implementation.segment_create
  //                  ( B::Parameter::Segment::Create {...}
  //                  , size
  //                  );
  //              }
  //            )
  //        };
  //
  //       read_write_access.visit
  //         ( storage_id
  //         , [&]<storage::is_implementation StorageImplementation>
  //             ( auto& storage_implementation
  //             )
  //           {
  //             return storage_implementation.segment_remove
  //               ( B::Parameter::Segment::Remove {...}
  //               , segment_id
  //               );
  //           }
  //        );
  //    }
  //
  //    // remove the storage manually, ad-hoc access token
  //    storages.read_write_access().remove
  //      ( storage_id
  //      );
  //
  template<typename StorageImplementations>
    using Storages
      = util::HeterogeneousMap<storage::ID, StorageImplementations>
    ;
}
