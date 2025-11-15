// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <fmt/format.h>
#include <functional>
#include <iov/iov.hpp>
#include <iterator>
#include <list>
#include <mcs/IOV_Database.hpp>
#include <mcs/IOV_Meta.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/Provider.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/iov_backend/provider/State.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/testing/iov_backend/IOV_Backend.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mcs::iov_backend
{
  namespace
  {
    struct RunningHeapStorage
    {
      using StorageImplementation = core::storage::implementation::Heap;
      template<typename ProviderEndpoint, typename Executor>
        RunningHeapStorage
          ( ProviderEndpoint provider_endpoint
          , Executor& executor
          )
            : _storage_id
              { std::invoke
                ( [&]
                  {
                    using Parameter = StorageImplementation::Parameter;
                    return std::visit
                      ( [&] (auto&& client)
                        { return client.add
                            ( Storage
                              { _storage->implementation_id()
                              , core::storage::make_parameter
                                  ( _parameter_create
                                  )
                              , util::ASIO::make_connectable
                                  (_storages_provider.local_endpoint())
                              , util::ASIO::make_connectable
                                  (asio::ip::tcp::endpoint{})
                              , _storage->id()
                              , core::storage::make_parameter
                                  ( Parameter::Size::Max{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::Size::Used{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::Segment::Create{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::Segment::Remove{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::Chunk::Description{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::File::Read{}
                                  )
                              , core::storage::make_parameter
                                  ( Parameter::File::Write{}
                                  )
                              }
                            );
                        }
                      , iov_backend::make_client
                            < rpc::access_policy::Exclusive
                            >
                        ( executor
                        , util::ASIO::make_connectable (provider_endpoint)
                        )
                      );
                  }
                )
              }
      {}

    private:
      core::Storages<SupportedStorageImplementations> _storages{};

      rpc::ScopedRunningIOContext _io_context_storages_provider
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        };

      core::control::Provider<asio::ip::tcp, SupportedStorageImplementations>
        _storages_provider
          { _io_context_storages_provider
          , asio::ip::tcp::endpoint{}
          , std::addressof (_storages)
          };

        typename StorageImplementation::Parameter::Create _parameter_create
          { core::storage::MaxSize::Unlimited{}
          };

      SupportedStorageImplementations::template wrap
        < core::UniqueStorage
        , StorageImplementation
        > _storage
          { core::make_unique_storage<StorageImplementation>
            ( std::addressof (_storages)
            , _parameter_create
            )
          };

      storage::ID _storage_id;
    };
  }

  namespace
  {
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::microseconds;

    template<typename Fun>
      auto timed (Fun&& fun)
    {
      auto const start {Clock::now()};
      std::invoke (std::forward<Fun> (fun));
      auto const end {Clock::now()};
      return std::chrono::duration_cast<Duration> (end - start);
    }
  }

  namespace
  {
    struct IOVBackendTimingRP
      : public testing::random::Test
      , public ::testing::WithParamInterface<std::tuple<std::size_t, std::size_t>>
    {};
  }

  TEST_P (IOVBackendTimingRP, collection_management)
  {
    auto const number_of_storages {std::get<0> (GetParam())};
    auto const number_of_collections {std::get<1> (GetParam())};

    auto io_context_provider
      { rpc::ScopedRunningIOContext
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    auto io_context_provider_storages_clients
      { rpc::ScopedRunningIOContext
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    auto storages_clients {provider::StoragesClients{}};

    auto const start_make_provider {Clock::now()};
    auto const provider
      { Provider<asio::ip::tcp, decltype (io_context_provider_storages_clients)>
          { asio::ip::tcp::endpoint{}
          , io_context_provider
          , io_context_provider_storages_clients
          , std::addressof (storages_clients)
          , provider::State{}
          }
      };
    auto const end_make_provider {Clock::now()};
    auto const duration_make_provider
      { std::chrono::duration_cast<Duration>
          ( end_make_provider - start_make_provider
          )
      };

    auto const start_make_storages {Clock::now()};
    auto const storages
      { std::invoke
        ( [&]
          {
            auto io_context_register
              { rpc::ScopedRunningIOContext
                { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
                , SIGINT, SIGTERM
                }
              };

            auto _storages {std::list<RunningHeapStorage>{}};

            for (auto i {std::size_t {0}}; i != number_of_storages; ++i)
            {
              _storages.emplace_back
                ( provider.local_endpoint()
                , io_context_register
                );
            }

            return _storages;
          }
        )
      };
    auto const end_make_storages {Clock::now()};
    auto const duration_make_storages
      { std::chrono::duration_cast<Duration>
          ( end_make_storages - start_make_storages
          )
      };

    auto iov_backend
      { testing::iov_backend::IOV_Backend
        { Parameter {util::ASIO::make_connectable (provider.local_endpoint())}
        }
      };
    auto database {IOV_Database{}};
    auto database_ref {IOV_DBRef {std::addressof (database)}};
    auto iov_backend_context {iov_backend.context (std::addressof (database))};

    auto const workspace_name {iov::meta::Name {"workspace-name"}};

    auto workspace
      { iov::Workspace
        { iov_backend::invoke_and_throw_on_unexpected
            ( iov::meta::MetaData::create_workspace
            , fmt::format
                ( "IOV: Could not create workspace '{}'"
                , workspace_name
                )
            , workspace_name
            , std::addressof (database_ref)
            )
        , iov::WorkspaceControls
            { iov::concurrency::Exclusive{}
            , iov::consistency::Sequential{}
            , iov::mutability::Immutable{}
            }
        , std::addressof (iov_backend_context)
        }
      };

    auto const collection_create
      { [&] (auto collection_name)
        {
          auto meta_data
            { iov_backend::invoke_and_throw_on_unexpected
              ( iov::meta::MetaData::create_collection
              , fmt::format
                  ( "Could not create collection '{}'"
                  , collection_name
                  )
              , collection_name
              , workspace_name
              , std::addressof (database_ref)
              )
            };

          auto const collection_open_result
            { iov_backend.collection_open
              ( iov::CollectionOpenParam
                { .name = iov::meta::Name {"ignored_by_backend_ops"}
                , .meta_data = std::addressof (meta_data)
                , .controls = iov::CollectionControls
                  { iov::concurrency::Exclusive{}
                  , iov::consistency::Sequential{}
                  , iov::mutability::Immutable{}
                  , iov::temporality::Once{}
                  , iov::Size_Ctl
                    { .request_size = iov::size::Size
                        { .min = iov::size::limit::Unknown{}
                        , .max = iov::size::limit::Unknown{}
                        }
                    , .collection_size = iov::size::Size
                        { .min = iov::size::limit::Unknown{}
                        , .max = iov::size::limit::Value {number_of_storages}
                        }
                    }
                  }
                , .workspace = workspace
                }
              )
            };

          if (!collection_open_result)
          {
            throw std::runtime_error
              { fmt::format ( "collection_open ({}): {}"
                            , collection_name
                            , collection_open_result.error().reason()
                            )
              };
          }
        }
      };

    auto collection_names {std::vector<iov::meta::Name>{}};
    collection_names.reserve (number_of_collections);
    std::generate_n
      ( std::back_inserter (collection_names)
      , number_of_collections
      , [n = 0]() mutable
        {
          return fmt::format ("{}", n++);
        }
      );

    auto const duration_create_collections
      { timed
        ( [&]
          {
            for (auto collection_name : collection_names)
            {
              collection_create (collection_name);
            }
          }
        )
      };

    ASSERT_GE ( database.size (database.read_access())
              , number_of_collections
              );

    for (auto collection_name : collection_names)
    {
      ASSERT_TRUE
        ( iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::exists_collection
          , fmt::format ("Exists collection {}", collection_name)
          , collection_name
          , workspace_name
          , std::addressof (database_ref)
          )
        );
    }

    std::ranges::shuffle (collection_names, testing::random::random_device());

    auto const collection_delete
      { [&] (auto collection_name)
        {
          auto meta_data
            { iov_backend::invoke_and_throw_on_unexpected
              ( iov::meta::MetaData::load_collection
              , fmt::format
                  ( "Could not load collection '{}'"
                  , collection_name
                  )
              , collection_name
              , workspace_name
              , std::addressof (database_ref)
              )
            };

          auto const collection_delete_result
            { iov_backend.collection_delete
              ( iov::DeleteParam
                { .name = iov::meta::Name {"ignored_by_backend_ops"}
                , .meta_data = std::addressof (meta_data)
                }
              ).get().result
            };

          if (!collection_delete_result)
          {
            throw std::runtime_error
              { fmt::format ( "collection_delete ({}): {}"
                            , collection_name
                            , collection_delete_result.error().reason()
                            )
              };
          }
        }
      };

    auto const duration_delete_collections
      { timed
        ( [&]
          {
            for (auto collection_name : collection_names)
            {
              collection_delete (collection_name);
            }
          }
        )
      };

    iov_backend::invoke_and_throw_on_unexpected
      ( iov::meta::MetaData::delete_workspace
      , fmt::format ("Could not delete workspace '{}'", workspace_name)
      , workspace_name
      , std::addressof (database_ref)
      );

    auto const database_contains_only_format_information
      { [&]() noexcept
        {
          return database.size (database.read_access()) == 1
              && database.contains
                   ( database.read_access()
                   , iov::meta::Key {"dbinfo:format"}
                   )
              ;
        }

      };
    ASSERT_TRUE (database_contains_only_format_information());

    fmt::print
      ( "number_of_storages {}"
        " number_of_collections {}"
        " make_provider {}"
        " make_storages {}"
        " create_collections {}"
        " delete_collections {}"
        "\n"
      , number_of_storages
      , number_of_collections
      , duration_make_provider
      , duration_make_storages
      , duration_create_collections
      , duration_delete_collections
      );
  }
  INSTANTIATE_TEST_SUITE_P
    ( IOVBackendTimingRP
    , IOVBackendTimingRP
    , ::testing::Combine
      ( ::testing::Values (1, 10, 100)
      , ::testing::Values (1, 10, 100, 1000, 10000)
      )
    );

}
