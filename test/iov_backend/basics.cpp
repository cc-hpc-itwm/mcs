// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <barrier>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iov/iov.hpp>
#include <iterator>
#include <mcs/IOV_Database.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/iov_backend/Provider.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/testing/UniqID.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/iov_backend/IOV_Backend.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/STD/byte.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/write_file.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/select.hpp>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#define MCS_TEST_IOV_BACKEND_ASSERT_HAS_VALUE(v...)     \
  if (!v)                                               \
  {                                                     \
    FAIL() << v.error().reason();                       \
  }                                                     \
  ASSERT_TRUE (v)

#define MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE(data, offset)      \
  do                                                                    \
  {                                                                     \
    auto const write_request_data                                       \
      { _iov_backend                                                    \
      . write ({_running_iov_backend.collection(), data, offset})       \
      . get()                                                           \
      };                                                                \
    MCS_TEST_IOV_BACKEND_ASSERT_HAS_VALUE (write_request_data.result);  \
    ASSERT_EQ (write_request_data.result.value(), std::size (data));    \
  } while (0)

#define MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ(data, offset)       \
  do                                                                    \
  {                                                                     \
    auto const read_request_data                                        \
      { _iov_backend                                                    \
      . read ({_running_iov_backend.collection(), data, offset})        \
      . get()                                                           \
      };                                                                \
    MCS_TEST_IOV_BACKEND_ASSERT_HAS_VALUE (read_request_data.result);   \
    ASSERT_EQ (read_request_data.result.value(), std::size (data));     \
  } while (0)

namespace mcs::iov_backend
{
  namespace
  {
    struct CreateDirectory
    {
      explicit CreateDirectory (std::filesystem::path);

      [[nodiscard]] operator std::filesystem::path() const;

    private:
      std::filesystem::path _path;
    };

    CreateDirectory::CreateDirectory (std::filesystem::path path)
      : _path {path}
    {
      std::filesystem::create_directory (_path);
    }

    CreateDirectory::operator std::filesystem::path() const
    {
      return _path;
    }
  }

  namespace
  {
    // \todo should that become part of testing::iov_backend?
    template<core::storage::is_implementation StorageImplementation>
      struct RunningStorage
    {
      template<typename Endpoint, typename... Args>
        RunningStorage
          ( core::memory::Size max_size
          , Endpoint provider_endpoint
          , Args&&... args
          )
            : _max_size {max_size}
            , _parameter_create
              { std::forward<Args> (args)...
              , core::storage::MaxSize::Limit {_max_size}
              }
            , _storage_id
              { std::invoke
                ( [&]
                  {
                    auto io_context_client
                      { rpc::ScopedRunningIOContext
                        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
                        , SIGINT, SIGTERM
                        }
                      };

                    using Parameter = typename StorageImplementation::Parameter;
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
                              , _transport_provider.connection_information()
                              , _storage->id()
                              , core::storage::make_parameter
                                  ( typename Parameter::Size::Max{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::Size::Used{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::Segment::Create{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::Segment::Remove{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::Chunk::Description{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::File::Read{}
                                  )
                              , core::storage::make_parameter
                                  ( typename Parameter::File::Write{}
                                  )
                              }
                            );
                        }
                      , iov_backend::make_client
                            < rpc::access_policy::Exclusive
                            >
                        ( io_context_client
                        , util::ASIO::make_connectable (provider_endpoint)
                        )
                      );
                  }
                )
              }
      {}

    private:
      core::memory::Size _max_size;

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

      typename StorageImplementation::Parameter::Create _parameter_create;

      SupportedStorageImplementations::template wrap
        < core::UniqueStorage
        , StorageImplementation
        > _storage
          { core::make_unique_storage<StorageImplementation>
            ( std::addressof (_storages)
            , _parameter_create
            )
          };

      rpc::ScopedRunningIOContext _io_context_transport_provider
        { rpc::ScopedRunningIOContext::NumberOfThreads {2u}
        , SIGINT, SIGTERM
        };

      using TransportProvider
        = core::transport::implementation::ASIO::Provider
            < asio::ip::tcp
            , SupportedStorageImplementations
            >
        ;
      TransportProvider _transport_provider
        { _io_context_transport_provider
        , asio::ip::tcp::endpoint{}
        , std::addressof (_storages)
        };

      storage::ID _storage_id;
    };

    // \todo should that become part of testing::iov_backend?
    struct RunningIOVBackend
    {
      RunningIOVBackend (std::size_t memory_size);

      [[nodiscard]] auto provider() const -> util::ASIO::AnyConnectable;

      [[nodiscard]] auto workspace() noexcept -> iov::Workspace*
      {
        return std::addressof (_workspace);
      }
      [[nodiscard]] auto collection() noexcept -> iov::Collection*
      {
        return std::addressof (_collection);
      }
      [[nodiscard]] auto meta_data() noexcept -> iov::meta::MetaData*
      {
        return std::addressof (_meta_data_collection);
      }

      // RunningIOVBackends take addresses of members.
      //
      RunningIOVBackend (RunningIOVBackend const&) = delete;
      RunningIOVBackend (RunningIOVBackend&&) = delete;
      auto operator= (RunningIOVBackend const&) -> RunningIOVBackend = delete;
      auto operator= (RunningIOVBackend&&) -> RunningIOVBackend = delete;
      ~RunningIOVBackend() = default;

    private:
      std::size_t _memory_size;

      rpc::ScopedRunningIOContext _io_context_provider
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        };

      rpc::ScopedRunningIOContext _io_context_provider_storages_clients
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        };

      provider::StoragesClients _storages_clients;

      Provider<asio::ip::tcp, decltype (_io_context_provider_storages_clients)>
        _provider { asio::ip::tcp::endpoint{}
                  , _io_context_provider
                  , _io_context_provider_storages_clients
                  , std::addressof (_storages_clients)
                  , provider::State{}
                  };

      testing::UniqTemporaryDirectory _temporary_directory
        { testing::UniqTemporaryDirectory
            {"MCS_TEST_IOV_BACKEND_RUNNING_IOV_BACKEND"}
        };

      RunningStorage<core::storage::implementation::SHMEM> _storage1
        { core::memory::make_size (_memory_size / 3)
        , _provider.local_endpoint()
        , core::storage::implementation::SHMEM::Prefix
            { util::string
              { testing::UniqID {"MCS_TEST_IOV_BACKEND_RUNNING_MCS_STORAGE1"}()
              }
            }
        };
      RunningStorage<core::storage::implementation::Heap> _storage2
        { core::memory::make_size (_memory_size / 3)
        , _provider.local_endpoint()
        };
      RunningStorage<core::storage::implementation::Files> _storage3
        { core::memory::make_size (_memory_size - 2 * (_memory_size / 3))
        , _provider.local_endpoint()
        , core::storage::implementation::Files::Prefix
          { CreateDirectory {_temporary_directory.path() / "storage3_data"}
          }
        };

      IOV_Database _database;

      iov::BackendContext _backend_context
        { .backend_id = iov::BackendContext::UUID {0}
        , .md_backend
            = iov::meta::MetadataBackend
              { std::unique_ptr<iov::meta::AbstractDatabaseBackend>
                  { new mcs::IOV_DBRef {std::addressof (_database)}
                  }
              }
        , .backend_implementation = iov::DLHandle {nullptr}
        , .ops = iov::BackendOps {iov::iov_backend_ops()}
        , .state = iov::BackendOps::State{}
        };

      iov::meta::Name _workspace_name {"workspace-name"};
      iov::meta::Name _collection_name {"collection-name"};

      iov::meta::MetaData _meta_data_workspace
        { iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::create_workspace
          , fmt::format ("Could not create workspace")
          , _workspace_name
          , _backend_context.md_backend.db.get()
          )
        };

      iov::Workspace _workspace
        { _meta_data_workspace
        , iov::WorkspaceControls
          { iov::concurrency::Exclusive{}
          , iov::consistency::Sequential{}
          , iov::mutability::Immutable{}
          }
        , std::addressof (_backend_context)
        };

      iov::meta::MetaData _meta_data_collection
        { iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::create_collection
          , fmt::format ("Could not create collection")
          , _collection_name
          , _workspace_name
          , _backend_context.md_backend.db.get()
          )
        };
      iov::Collection _collection
        { _workspace
        , _meta_data_collection
        , iov::CollectionControls
          { iov::concurrency::Exclusive{}
          , iov::consistency::Sequential{}
          , iov::mutability::Immutable{}
          , iov::temporality::Once{}
          , iov::Size_Ctl
            { .request_size
                = iov::size::Size
                  { .min = iov::size::limit::Unknown{}
                  , .max = iov::size::limit::Unknown{}
                  }
            , .collection_size
                = iov::size::Size
                  { .min = iov::size::limit::Unknown{}
                  , .max = iov::size::limit::Unknown{}
                  }
            }
          }
        };
    };

    RunningIOVBackend::RunningIOVBackend
      ( std::size_t memory_size
      )
        : _memory_size {memory_size}
    {}

    auto RunningIOVBackend::provider() const -> util::ASIO::AnyConnectable
    {
      return util::ASIO::make_connectable (_provider.local_endpoint());
    }
  }

  namespace
  {
    [[nodiscard]] auto make_mutable_span
      ( std::vector<std::byte>& data
      ) -> std::span<std::byte>
    {
      return std::span<std::byte>
        { data.data()
        , data.data() + data.size() * sizeof (std::byte)
        };
    }

    [[nodiscard]] auto make_const_span
      ( std::vector<std::byte> const& data
      ) -> std::span<std::byte const>
    {
      return std::span<std::byte const>
        { data.data()
        , data.data() + data.size() * sizeof (std::byte)
        };
    }
  }

  namespace
  {
    auto generate_random_data (std::span<std::byte> range)
    {
      std::ranges::generate (range, testing::random::value<std::byte>{});
    }

    auto random_data (std::size_t size) -> std::vector<std::byte>
    {
      auto data {std::vector<std::byte> (size)};

      generate_random_data (data);

      return data;
    }
  }

  namespace
  {
    struct RunningIOVBackendR : public testing::random::Test
    {
      using RandomOffT = testing::random::value<off_t>;
      using RandomDataSize = testing::random::value<std::size_t>;

      std::size_t const _memory_size
        { // at least one byte for each of the three storages, divisible by three
          3 * RandomDataSize
              { RandomDataSize::Min {1}
              , RandomDataSize::Max {1 << 20}
              }()
        };
      RunningIOVBackend _running_iov_backend {_memory_size};

      iov::meta::Name _collection_name {"RunningIOVBackendR_collection"};

      template<typename IOVBackend>
        auto collection_create
          ( IOVBackend& iov_backend
          , iov::size::limit::Size collection_max_size
          )
      {
        auto const collection_open_result
          { iov_backend.collection_open
            ( iov::CollectionOpenParam
              { .name = _collection_name
              , .meta_data = _running_iov_backend.meta_data()
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
                      , .max = collection_max_size
                      }
                  }
                }
              , .workspace = *_running_iov_backend.workspace()
              }
            )
          };

        if (!collection_open_result)
        {
          throw std::runtime_error
            { fmt::format ( "collection_open: {}"
                          , collection_open_result.error().reason()
                          )
            };
        }

        collection_close (iov_backend);
      }

      template<typename IOVBackend>
        auto collection_create
          ( IOVBackend& iov_backend
          )
      {
        return collection_create
          ( iov_backend
          , iov::size::limit::Value {_memory_size}
          );
      }

      template<typename IOVBackend>
        auto collection_close (IOVBackend& iov_backend)
      {
        auto const collection_close_result
          { iov_backend.collection_close
            ( iov::CollectionCloseParam
              { .name = _collection_name
              , .meta_data = _running_iov_backend.meta_data()
              }
            )
          };

        if (!collection_close_result)
        {
          fmt::print ( "collection_close: {}"
                     , collection_close_result.error().reason()
                     );
        }
      }

      template<typename IOVBackend>
        auto collection_delete (IOVBackend& iov_backend)
      {
        collection_close (iov_backend);
        auto const collection_delete_result
          { iov_backend.collection_delete
            ( iov::DeleteParam
              { .name = _collection_name
              , .meta_data = _running_iov_backend.meta_data()
              }
            ).get()
          };

        if (!collection_delete_result.result)
        {
          // dtor must not throw
          fmt::print ( "collection_delete: {}"
                     , collection_delete_result.result.error().reason()
                     );
        }
      }
    };
  }

  namespace
  {
    struct IOV_BackendR : public RunningIOVBackendR
    {
      IOV_BackendR()
      {
        collection_create (_iov_backend);
      }
      ~IOV_BackendR() override
      {
        util::execute_and_die_on_exception
          ( "~IOV_BackendR"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          );
      }
      IOV_BackendR (IOV_BackendR const&) = delete;
      IOV_BackendR (IOV_BackendR&&) = delete;
      auto operator= (IOV_BackendR const&) -> IOV_BackendR = delete;
      auto operator= (IOV_BackendR&&) -> IOV_BackendR = delete;

      testing::iov_backend::IOV_Backend _iov_backend
        { Parameter {_running_iov_backend.provider()}
        };
    };
  }

  TEST_F (IOV_BackendR, to_read_before_zero_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const negative_offset {RandomOffT {RandomOffT::Max {-1}}()};
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , negative_offset
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format ("cast signed to unsigned: {} < 0", negative_offset)
      );
  }

  TEST_F (IOV_BackendR, to_read_after_end_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const offset_after_end
      { RandomOffT {RandomOffT::Min {util::cast<off_t> (_memory_size)}}()
      };
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , offset_after_end
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format
        ( "mcs::iov_backend::read: "
          "range {} is not inside the memory range {}"
        , core::memory::make_range
          ( core::memory::make_offset (offset_after_end)
          , core::memory::make_size (data_size)
          )
        , core::memory::make_range
          ( core::memory::make_offset (0)
          , core::memory::make_size (_memory_size)
          )
        )
      );
  }

  TEST_F (IOV_BackendR, to_read_beyond_end_is_an_error)
  {
    auto const inner_offset
      { RandomOffT
        { RandomOffT::Min {1}
        , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
        }()
      };
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min
          { _memory_size - util::cast<std::size_t> (inner_offset) + 1
          }
        , RandomDataSize::Max
          { _memory_size
          }
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , inner_offset
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format
        ( "mcs::iov_backend::read: "
          "range {} is not inside the memory range {}"
        , core::memory::make_range
          ( core::memory::make_offset (inner_offset)
          , core::memory::make_size (data_size)
          )
        , core::memory::make_range
          ( core::memory::make_offset (0)
          , core::memory::make_size (_memory_size)
          )
        )
      );
  }

  TEST_F (IOV_BackendR, to_write_before_zero_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto const data {std::vector<std::byte> (data_size)};
    auto const negative_offset {RandomOffT {RandomOffT::Max {-1}}()};
    auto const write_parameter
      { iov::WriteParam
          { _running_iov_backend.collection()
          , make_const_span (data)
          , negative_offset
          }
      };
    auto const write_request_data {_iov_backend.write (write_parameter).get()};

    ASSERT_FALSE (write_request_data.result);
    ASSERT_EQ
      ( write_request_data.result.error().reason()
      , fmt::format ("cast signed to unsigned: {} < 0", negative_offset)
      );
  }

  TEST_F (IOV_BackendR, to_write_after_max_size_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto const data {std::vector<std::byte> (data_size)};
    auto const offset_after_capacity
      { RandomOffT
        { RandomOffT::Min {util::cast<off_t> (_memory_size)}
        }()
      };
    auto const write_parameter
      { iov::WriteParam
          { _running_iov_backend.collection()
          , make_const_span (data)
          , offset_after_capacity
          }
      };
    auto const write_request_data {_iov_backend.write (write_parameter).get()};

    ASSERT_FALSE (write_request_data.result);
    ASSERT_EQ
      ( write_request_data.result.error().reason()
      , fmt::format
        ( "mcs_iov_backend: write after max size '{}' of collection '{}'"
        , _memory_size
        , mcs::iov_backend::collection::ID
          { _running_iov_backend.collection()->metadata().uuid().str()
          }
        )
      );
  }

  TEST_F (IOV_BackendR, to_write_beyond_max_size_is_an_error)
  {
    auto const inner_offset
      { RandomOffT
        { RandomOffT::Min {1}
        , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
        }()
      };
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min
          { _memory_size - util::cast<std::size_t> (inner_offset) + 1
          }
        , RandomDataSize::Max
          { _memory_size
          }
        }()
      };
    auto const data {std::vector<std::byte> (data_size)};
    auto const write_parameter
      { iov::WriteParam
          { _running_iov_backend.collection()
          , make_const_span (data)
          , inner_offset
          }
      };
    auto const write_request_data {_iov_backend.write (write_parameter).get()};

    ASSERT_FALSE (write_request_data.result);
    ASSERT_EQ
      ( write_request_data.result.error().reason()
      , fmt::format
        ( "mcs_iov_backend: write after max size '{}' of collection '{}'"
        , _memory_size
        , mcs::iov_backend::collection::ID
          { _running_iov_backend.collection()->metadata().uuid().str()
          }
        )
      );
  }

  TEST_F (IOV_BackendR, to_read_written_complete_memory_is_identity)
  {
    auto const original_data {random_data (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , 0
      );

    auto retrieved_data {std::vector<std::byte> (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_EQ (original_data, retrieved_data);
  }

  TEST_F ( IOV_BackendR
         , to_read_into_mcs_buffer_written_complete_memory_is_identity
         )
  {
    auto const original_data {random_data (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , 0
      );

    auto retrieved_data {_iov_backend.allocate (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , 0
      );

    ASSERT_THAT (original_data, ::testing::ElementsAreArray (*retrieved_data));
  }

  TEST_F ( IOV_BackendR
         , to_read_written_from_mcs_buffer_complete_memory_is_identity
         )
  {
    auto original_data {_iov_backend.allocate (_memory_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , 0
      );

    auto retrieved_data {std::vector<std::byte> (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_THAT (retrieved_data, ::testing::ElementsAreArray (*original_data));
  }

  namespace
  {
    struct IOV_BackendMaxSizeUnknownR : public RunningIOVBackendR
    {
      IOV_BackendMaxSizeUnknownR()
      {
        collection_create (_iov_backend, iov::size::limit::Unknown{});
      }
      ~IOV_BackendMaxSizeUnknownR() override
      {
        util::execute_and_die_on_exception
          ( "~IOV_BackendMaxSizeUnknownR"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          );
      }
      IOV_BackendMaxSizeUnknownR (IOV_BackendMaxSizeUnknownR const&) = delete;
      IOV_BackendMaxSizeUnknownR (IOV_BackendMaxSizeUnknownR&&) = delete;
      auto operator= (IOV_BackendMaxSizeUnknownR const&) -> IOV_BackendMaxSizeUnknownR = delete;
      auto operator= (IOV_BackendMaxSizeUnknownR&&) -> IOV_BackendMaxSizeUnknownR = delete;

      testing::iov_backend::IOV_Backend _iov_backend
        { Parameter {_running_iov_backend.provider()}
        };
    };
  }

  TEST_F (IOV_BackendMaxSizeUnknownR, to_read_before_zero_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const negative_offset {RandomOffT {RandomOffT::Max {-1}}()};
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , negative_offset
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format ("cast signed to unsigned: {} < 0", negative_offset)
      );
  }

  TEST_F (IOV_BackendMaxSizeUnknownR, to_read_after_end_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const offset_after_end
      { RandomOffT {RandomOffT::Min {util::cast<off_t> (_memory_size)}}()
      };
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , offset_after_end
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format
        ( "mcs::iov_backend::read: "
          "range {} is not inside the memory range {}"
        , core::memory::make_range
          ( core::memory::make_offset (offset_after_end)
          , core::memory::make_size (data_size)
          )
        , core::memory::make_range
          ( core::memory::make_offset (0)
          , core::memory::make_size (0)
          )
        )
      );
  }

  TEST_F (IOV_BackendMaxSizeUnknownR, to_read_beyond_end_is_an_error)
  {
    auto const inner_offset
      { RandomOffT
        { RandomOffT::Min {1}
        , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
        }()
      };
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min
          { _memory_size - util::cast<std::size_t> (inner_offset) + 1
          }
        , RandomDataSize::Max
          { _memory_size
          }
        }()
      };
    auto data {std::vector<std::byte> (data_size)};
    auto const read_parameter
      { iov::ReadParam
          { _running_iov_backend.collection()
          , make_mutable_span (data)
          , inner_offset
          }
      };
    auto const read_request_data {_iov_backend.read (read_parameter).get()};

    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ
      ( read_request_data.result.error().reason()
      , fmt::format
        ( "mcs::iov_backend::read: "
          "range {} is not inside the memory range {}"
        , core::memory::make_range
          ( core::memory::make_offset (inner_offset)
          , core::memory::make_size (data_size)
          )
        , core::memory::make_range
          ( core::memory::make_offset (0)
          , core::memory::make_size (0)
          )
        )
      );
  }

  TEST_F (IOV_BackendMaxSizeUnknownR, to_write_before_zero_is_an_error)
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {1 << 20}
        }()
      };
    auto const data {std::vector<std::byte> (data_size)};
    auto const negative_offset {RandomOffT {RandomOffT::Max {-1}}()};
    auto const write_parameter
      { iov::WriteParam
          { _running_iov_backend.collection()
          , make_const_span (data)
          , negative_offset
          }
      };
    auto const write_request_data {_iov_backend.write (write_parameter).get()};

    ASSERT_FALSE (write_request_data.result);
    ASSERT_EQ
      ( write_request_data.result.error().reason()
      , fmt::format ("cast signed to unsigned: {} < 0", negative_offset)
      );
  }

  TEST_F ( IOV_BackendMaxSizeUnknownR
         , the_first_write_at_zero_grows_the_collection
         )
  {
    auto const data_size
      { RandomDataSize
        { RandomDataSize::Min {0}
        , RandomDataSize::Max {_memory_size}
        }()
      };
    auto const original_data {random_data (data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE (original_data, off_t {0});

    auto retrieved_data {std::vector<std::byte> (data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_EQ (original_data, retrieved_data);
  }

  namespace
  {
    struct MaximumNumberOfParts
    {
      [[nodiscard]] explicit constexpr MaximumNumberOfParts (std::size_t value)
        : _value {value}
      {
        if (std::cmp_less (_value, 1))
        {
          throw std::logic_error
            { "MaximumNumberOfParts: At least 1 part is required."
            };
        }
      }
      [[nodiscard]] constexpr auto value() const noexcept
      {
        return _value;
      }

    private:
      std::size_t _value;
    };

    template<typename Span, typename UsePart>
      requires (std::invocable<UsePart, Span, std::size_t>)
      auto for_each_part_of_a_random_ascending_touching_prefix
        ( Span span
        , MaximumNumberOfParts max_number_of_parts
        , UsePart use_part
        ) -> std::size_t
    {
      auto const part_ends
        { std::invoke
          ( [&]
            {
              using RandomNumber = testing::random::value<std::size_t>;
              auto const number_of_parts
                { std::invoke
                  ( RandomNumber
                    { RandomNumber::Min {1}
                    , RandomNumber::Max
                      { std::min (max_number_of_parts.value(), span.size())
                      }
                    }
                  )
                };
              using RandomPartEnd = testing::random::value<std::size_t>;
              auto random_part_end
                { RandomPartEnd
                  { RandomPartEnd::Min {1}
                  , RandomPartEnd::Max {span.size()}
                  }
                };
              auto _part_ends
                { testing::random::unique_values<std::size_t>
                  ( random_part_end
                  , number_of_parts
                  )
                };
              std::ranges::sort (_part_ends);

              return _part_ends;
            }
          )
        };

      auto part_begin {std::size_t {0}};

      std::ranges::for_each
        ( part_ends
        , [&] (auto part_end)
          {
            auto const part_size {part_end - part_begin};

            std::invoke
              ( use_part
              , span.subspan (part_begin, part_size)
              , part_begin
              );

            part_begin += part_size;
          }
        );

      return part_begin;
    }

    template<typename Span, typename UsePart>
      requires (std::invocable<UsePart, Span, std::size_t>)
      auto for_each_part_of_a_random_ascending_touching_cover
        ( Span span
        , MaximumNumberOfParts max_number_of_parts
        , UsePart use_part
        ) -> void
    {
      if (std::cmp_less (max_number_of_parts.value(), 2))
      {
        std::invoke (use_part, span, std::size_t {0});

        return;
      }

      auto const prefix_size
        ( for_each_part_of_a_random_ascending_touching_prefix
          ( span
          , MaximumNumberOfParts {max_number_of_parts.value() - 1}
          , use_part
          )
        );

      if (prefix_size < span.size())
      {
        auto const remaining {span.size() - prefix_size};

        std::invoke
          ( use_part
          , span.subspan (prefix_size, remaining)
          , prefix_size
          );
      }
    }
  }

  TEST_F ( IOV_BackendMaxSizeUnknownR
         , to_write_parts_in_ascending_consecutive_touching_order_starting_at_zero_results_in_all_data_being_written
         )
  {
    auto const original_data {random_data (_memory_size)};

    for_each_part_of_a_random_ascending_touching_cover
      ( std::span {original_data}
      , MaximumNumberOfParts {std::size_t {100}}
      , [&] (auto part, auto part_begin)
        {
          MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
            ( part, util::cast<off_t> (part_begin)
            );
        }
      );

    auto retrieved_data {std::vector<std::byte> (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_EQ (original_data, retrieved_data);
  }

  TEST_F ( IOV_BackendMaxSizeUnknownR
         , to_write_parts_in_any_order_results_in_all_data_being_written
         )
  {
    auto const original_data {random_data (_memory_size)};

    struct Part
    {
      std::span<std::byte const> data;
      off_t offset;
    };

    auto const parts
      { std::invoke
        ( [&]
          {
            auto _parts {std::vector<Part>{}};

            for_each_part_of_a_random_ascending_touching_cover
              ( std::span {original_data}
              , MaximumNumberOfParts {std::size_t {100}}
              , [&] (auto part, auto part_begin)
                {
                  _parts.emplace_back
                    ( part
                    , util::cast<off_t> (part_begin)
                    );
                }
              );

            std::ranges::shuffle (_parts, testing::random::random_device());

            return _parts;
          }
        )
      };

    std::ranges::for_each
      ( parts
      , [&] (auto part)
        {
          MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE (part.data, part.offset);
        }
      );

    auto retrieved_data {std::vector<std::byte> (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_EQ (original_data, retrieved_data);
  }

  TEST_F ( IOV_BackendMaxSizeUnknownR
         , to_write_parts_in_any_order_by_multiple_clients_results_in_all_data_being_written
         )
  {
    auto const original_data {random_data (_memory_size)};

    struct Part
    {
      std::span<std::byte const> data;
      off_t offset;
    };

    auto const parts
      { std::invoke
        ( [&]
          {
            auto _parts {std::vector<Part>{}};

            for_each_part_of_a_random_ascending_touching_cover
              ( std::span {original_data}
              , MaximumNumberOfParts {std::size_t {100}}
              , [&] (auto part, auto part_begin)
                {
                  _parts.emplace_back
                    ( part
                    , util::cast<off_t> (part_begin)
                    );
                }
              );

            std::ranges::shuffle (_parts, testing::random::random_device());

            return _parts;
          }
        )
      };

    struct Result
    {
      struct OK{};
      struct Error{};
      struct WrongNumberOfBytes{};
    };
    using WriterResult = std::variant< Result::OK
                                     , Result::Error
                                     , Result::WrongNumberOfBytes
                                     >;

    auto writers {std::vector<std::future<WriterResult>>{}};
    writers.reserve (parts.size());

    auto barrier {std::barrier {util::cast<std::ptrdiff_t> (parts.size())}};

    std::ranges::for_each
      ( parts
      , [&] (auto part)
        {
          writers.emplace_back
            ( std::async
              ( std::launch::async
              , [&, part]() -> WriterResult
                {
                  auto iov_backend
                    { testing::iov_backend::IOV_Backend
                      { Parameter {_running_iov_backend.provider()}
                      }
                    };

                  barrier.arrive_and_wait();

                  auto const write_request_data
                    { iov_backend
                    . write ( { _running_iov_backend.collection()
                              , part.data
                              , part.offset
                              }
                            )
                    . get()
                    };

                  if (!write_request_data.result)
                  {
                    return Result::Error{};
                  }

                  if ( std::cmp_not_equal
                       ( write_request_data.result.value()
                       , std::size (part.data)
                       )
                     )
                  {
                    return Result::WrongNumberOfBytes{};
                  }

                  return Result::OK{};
                }
              )
            );
        }
      );

    struct ResultCount
    {
      unsigned _ok {0U};
      unsigned _error {0U};
      unsigned _wrong_number_of_bytes {0U};
    };

    auto const result_count
      { std::accumulate
        ( std::begin (writers), std::end (writers)
        , ResultCount{}
        , [] (auto count, auto& writer)
          {
            std::visit
              ( mcs::util::overloaded
                ( [&] (Result::OK) noexcept
                  {
                    count._ok += 1U;
                  }
                , [&] (Result::Error) noexcept
                  {
                    count._error += 1U;
                  }
                , [&] (Result::WrongNumberOfBytes) noexcept
                  {
                    count._wrong_number_of_bytes += 1U;
                  }
                )
              , writer.get()
              );

            return count;
          }
        )
      };

    EXPECT_EQ (result_count._error, 0U);
    EXPECT_EQ (result_count._wrong_number_of_bytes, 0U);
    ASSERT_EQ (result_count._ok, parts.size());

    auto retrieved_data {std::vector<std::byte> (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , 0
      );

    ASSERT_EQ (original_data, retrieved_data);
  }

  namespace
  {
    struct IOV_BackendRandomPartR : public IOV_BackendR
    {
      off_t const _data_offset
        { RandomOffT
          { RandomOffT::Min {0}
          , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
          }()
        };
      std::size_t const _data_size
        { RandomDataSize
          { RandomDataSize::Min {1}
          , RandomDataSize::Max
              {_memory_size - util::cast<std::size_t> (_data_offset)}
          }()
        };
    };
  }

  TEST_F ( IOV_BackendRandomPartR
         , to_read_direct_directly_written_random_part_is_identity
         )
  {
    ASSERT_LE
      ( util::cast<std::size_t> (_data_offset) + _data_size
      , _memory_size
      );

    // 0. fill the complete memory with some random data
    auto const original_data {random_data (_memory_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , 0
      );

    // 1. create random data for the part and write it
    auto const inserted_data {random_data (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (inserted_data)
      , _data_offset
      );

    // 2. retrieve the modified part and check it has the correct data
    {
      auto retrieved_data {std::vector<std::byte> (_data_size)};

      MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
        ( make_mutable_span (retrieved_data)
        , _data_offset
        );

      ASSERT_EQ (inserted_data, retrieved_data);
    }

    // 3. retrieve the complete space and check the untouched data
    // has not changed
    {
      auto retrieved_data {std::vector<std::byte> (_memory_size)};

      MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
        ( make_mutable_span (retrieved_data)
        , 0
        );

      auto const expected_data
        { std::invoke
          ( [&]
            {
              auto _expected_data {std::vector<std::byte>{}};
              _expected_data.insert
                ( std::end (_expected_data)
                , std::begin (original_data)
                , std::begin (original_data) + _data_offset
                );
              _expected_data.insert
                ( std::end (_expected_data)
                , std::begin (inserted_data)
                , std::end (inserted_data)
                );
              _expected_data.insert
                ( std::end (_expected_data)
                , std::begin (original_data) + _data_offset
                                             + util::cast<off_t> (_data_size)
                , std::end (original_data)
                );
              return _expected_data;
            }
          )
        };

      ASSERT_EQ (expected_data, retrieved_data);
    }
  }

  TEST_F ( IOV_BackendRandomPartR
         , to_read_indirect_indirectly_written_random_part_is_identity
         )
  {
    ASSERT_LE
      ( util::cast<std::size_t> (_data_offset) + _data_size
      , _memory_size
      );


    auto original_data {_iov_backend.allocate (_data_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , _data_offset
      );

    auto retrieved_data {_iov_backend.allocate (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , _data_offset
      );

    ASSERT_THAT (*original_data, ::testing::ElementsAreArray (*retrieved_data));
  }

  TEST_F ( IOV_BackendRandomPartR
         , to_read_direct_indirectly_written_random_part_is_identity
         )
  {
    ASSERT_LE
      ( util::cast<std::size_t> (_data_offset) + _data_size
      , _memory_size
      );


    auto original_data {_iov_backend.allocate (_data_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , _data_offset
      );

    auto retrieved_data {std::vector<std::byte> (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , _data_offset
      );

    ASSERT_THAT (*original_data, ::testing::ElementsAreArray (retrieved_data));
  }

  TEST_F ( IOV_BackendRandomPartR
         , to_read_indirect_directly_written_random_part_is_identity
         )
  {
    ASSERT_LE
      ( util::cast<std::size_t> (_data_offset) + _data_size
      , _memory_size
      );


    auto const original_data {random_data (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , _data_offset
      );

    auto retrieved_data {_iov_backend.allocate (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , _data_offset
      );

    ASSERT_THAT (original_data, ::testing::ElementsAreArray (*retrieved_data));
  }

  TEST_F ( RunningIOVBackendR
         , indirect_communication_with_volume_larger_than_size_of_communication_buffer_works
         )
  {
    auto const maximum_transfer_size
      { RandomDataSize
        { RandomDataSize::Min {1}
        , RandomDataSize::Max {_memory_size / 3}
        }()
      };

    auto _iov_backend
      { testing::iov_backend::IOV_Backend
        {  Parameter {_running_iov_backend.provider()}
        << Parameter::IndirectCommunication
           { ._number_of_buffers = 1
           , ._maximum_transfer_size
               = core::memory::make_size (maximum_transfer_size)
           , ._acquire_buffer_timeout_in_milliseconds = 0
           , ._maximum_number_of_parallel_streams = 1
           }
        }
      };

    collection_create (_iov_backend);

    auto const _collection_delete
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "collection_delete"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          )
      };

    auto const _data_size
      { RandomDataSize
        { RandomDataSize::Min {maximum_transfer_size + 1}
        , RandomDataSize::Max {_memory_size}
        }()
      };
    auto const _data_offset
      { RandomOffT
        { RandomOffT::Min {0}
        , RandomOffT::Max {util::cast<off_t> (_memory_size - _data_size)}
        }()
      };

    ASSERT_GT (_data_size, maximum_transfer_size);
    ASSERT_LE (_data_size, _memory_size);

    auto const original_data {random_data (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , _data_offset
      );

    auto retrieved_data {std::vector<std::byte> (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , _data_offset
      );

    ASSERT_EQ (original_data, retrieved_data);
  }


  namespace
  {
    struct IOV_BackendWithSmallIndirectCommunicationTimeoutR
      : public RunningIOVBackendR
    {
      IOV_BackendWithSmallIndirectCommunicationTimeoutR()
      {
        collection_create (_iov_backend);
      }
      ~IOV_BackendWithSmallIndirectCommunicationTimeoutR() override
      {
        util::execute_and_die_on_exception
          ( "~IOV_BackendWithSmallIndirectCommunicationTimeoutR"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          );
      }
      IOV_BackendWithSmallIndirectCommunicationTimeoutR (IOV_BackendWithSmallIndirectCommunicationTimeoutR const&) = delete;
      IOV_BackendWithSmallIndirectCommunicationTimeoutR (IOV_BackendWithSmallIndirectCommunicationTimeoutR&&) = delete;
      auto operator= (IOV_BackendWithSmallIndirectCommunicationTimeoutR const&) -> IOV_BackendWithSmallIndirectCommunicationTimeoutR = delete;
      auto operator= (IOV_BackendWithSmallIndirectCommunicationTimeoutR&&) -> IOV_BackendWithSmallIndirectCommunicationTimeoutR = delete;

      using RandomTimeout = testing::random::value<std::size_t>;
      testing::iov_backend::IOV_Backend _iov_backend
        {  Parameter {_running_iov_backend.provider()}
        << Parameter::IndirectCommunication
           { ._number_of_buffers = 0
           , ._maximum_transfer_size
               = testing::random::value<core::memory::Size>{}()
           , ._acquire_buffer_timeout_in_milliseconds
               = RandomTimeout
                 { RandomTimeout::Min {0}
                 , RandomTimeout::Max {9}
                 }()
           , ._maximum_number_of_parallel_streams = 1
           }
        };
      off_t const _data_offset
        { RandomOffT
          { RandomOffT::Min {0}
          , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
          }()
        };
      std::size_t const _data_size
        { RandomDataSize
          { RandomDataSize::Min {1}
          , RandomDataSize::Max
              {_memory_size - util::cast<std::size_t> (_data_offset)}
          }()
        };
    };
  }

  TEST_F ( IOV_BackendWithSmallIndirectCommunicationTimeoutR
         , indirect_write_without_communication_buffer_results_in_timeout
         )
  {
    auto const data {random_data (_data_size)};

    auto const write_request_data
      { _iov_backend.write
          ( { _running_iov_backend.collection()
            , make_const_span (data)
            , _data_offset
            }
          )
      . get()
      };
    ASSERT_FALSE (write_request_data.result);
    ASSERT_EQ ( write_request_data.result.error().reason()
              , "iov_backend::Bitmap::Set::Timeout"
              );
  }

  TEST_F ( IOV_BackendWithSmallIndirectCommunicationTimeoutR
         , indirect_read_without_communication_buffer_results_in_timeout
         )
  {
    auto data {std::vector<std::byte> (_data_size)};

    auto const read_request_data
      { _iov_backend.read
          ( { _running_iov_backend.collection()
            , make_mutable_span (data)
            , _data_offset
            }
          )
      . get()
      };
    ASSERT_FALSE (read_request_data.result);
    ASSERT_EQ ( read_request_data.result.error().reason()
              , "iov_backend::Bitmap::Set::Timeout"
              );
  }

  namespace
  {
    struct IOV_BackendWithSmallMaximumTransferUnitR
      : public RunningIOVBackendR
    {
      IOV_BackendWithSmallMaximumTransferUnitR()
      {
        collection_create (_iov_backend);
      }
      ~IOV_BackendWithSmallMaximumTransferUnitR() override
      {
        util::execute_and_die_on_exception
          ( "~IOV_BackendWithSmallMaximumTransferUnitR"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          );
      }
      IOV_BackendWithSmallMaximumTransferUnitR (IOV_BackendWithSmallMaximumTransferUnitR const&) = delete;
      IOV_BackendWithSmallMaximumTransferUnitR (IOV_BackendWithSmallMaximumTransferUnitR&&) = delete;
      auto operator= (IOV_BackendWithSmallMaximumTransferUnitR const&) -> IOV_BackendWithSmallMaximumTransferUnitR = delete;
      auto operator= (IOV_BackendWithSmallMaximumTransferUnitR&&) -> IOV_BackendWithSmallMaximumTransferUnitR = delete;

      using RandomMaximumTransferSize
        = testing::random::value<std::size_t>
        ;
      std::size_t _maximum_transfer_size
        { RandomMaximumTransferSize
          { RandomMaximumTransferSize::Min {1}
          , RandomMaximumTransferSize::Max {_memory_size / 3}
          }()
        };

      testing::iov_backend::IOV_Backend _iov_backend
        {  Parameter {_running_iov_backend.provider()}
        << Parameter::IndirectCommunication
           { ._number_of_buffers = 1 << 10
           , ._maximum_transfer_size
               = core::memory::make_size (_maximum_transfer_size)
           , ._acquire_buffer_timeout_in_milliseconds = 10'000
           , ._maximum_number_of_parallel_streams = 4
           }
        << Parameter::DirectCommunication
           { ._maximum_transfer_size
               = core::memory::make_size (_maximum_transfer_size)
           , ._maximum_number_of_parallel_streams = 4
           }
        };
      std::size_t _data_size
        { RandomDataSize
            { RandomDataSize::Min {_maximum_transfer_size + 1}
            , RandomDataSize::Max {_memory_size}
            }()
        };
      off_t _data_offset
        { RandomOffT
          { RandomOffT::Min {0}
          , RandomOffT::Max {util::cast<off_t> (_memory_size - _data_size)}
          }()
        };
    };
  }

  TEST_F ( IOV_BackendWithSmallMaximumTransferUnitR
         , direct_direct_communication_larger_than_maximum_transfer_size_works
         )
  {
    auto const original_data {random_data (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , _data_offset
      );

    auto retrieved_data {std::vector<std::byte> (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , _data_offset
      );

    ASSERT_EQ (retrieved_data, original_data);
  }

  TEST_F ( IOV_BackendWithSmallMaximumTransferUnitR
         , direct_indirect_communication_larger_than_maximum_transfer_size_works
         )
  {
    auto const original_data {random_data (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( make_const_span (original_data)
      , _data_offset
      );

    auto retrieved_data {_iov_backend.allocate (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , _data_offset
      );

    ASSERT_THAT (*retrieved_data, ::testing::ElementsAreArray (original_data));
  }

  TEST_F ( IOV_BackendWithSmallMaximumTransferUnitR
         , indirect_direct_communication_larger_than_maximum_transfer_size_works
         )
  {
    auto original_data {_iov_backend.allocate (_data_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , _data_offset
      );

    auto retrieved_data {std::vector<std::byte> (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( make_mutable_span (retrieved_data)
      , _data_offset
      );

    ASSERT_THAT (retrieved_data, ::testing::ElementsAreArray (*original_data));
  }

  TEST_F ( IOV_BackendWithSmallMaximumTransferUnitR
         , indirect_indirect_communication_larger_than_maximum_transfer_size_works
         )
  {
    auto original_data {_iov_backend.allocate (_data_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , _data_offset
      );

    auto retrieved_data {_iov_backend.allocate (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , _data_offset
      );

    ASSERT_THAT (*retrieved_data, ::testing::ElementsAreArray (*original_data));
  }

  namespace
  {
    struct IOV_BackendWithNoCommunicationBufferR
      : public RunningIOVBackendR
    {
      IOV_BackendWithNoCommunicationBufferR()
      {
        collection_create (_iov_backend);
      }
      ~IOV_BackendWithNoCommunicationBufferR() override
      {
        util::execute_and_die_on_exception
          ( "~IOV_BackendWithNoCommunicationBufferR"
          , [&]
            {
              collection_delete (_iov_backend);
            }
          );
      }
      IOV_BackendWithNoCommunicationBufferR (IOV_BackendWithNoCommunicationBufferR const&) = delete;
      IOV_BackendWithNoCommunicationBufferR (IOV_BackendWithNoCommunicationBufferR&&) = delete;
      auto operator= (IOV_BackendWithNoCommunicationBufferR const&) -> IOV_BackendWithNoCommunicationBufferR = delete;
      auto operator= (IOV_BackendWithNoCommunicationBufferR&&) -> IOV_BackendWithNoCommunicationBufferR = delete;

      using RandomSize = testing::random::value<std::size_t>;
      testing::iov_backend::IOV_Backend _iov_backend
        {  Parameter {_running_iov_backend.provider()}
        << Parameter::IndirectCommunication
           { ._number_of_buffers = 0
           , ._maximum_transfer_size = core::memory::make_size (RandomSize{}())
           , ._acquire_buffer_timeout_in_milliseconds = RandomSize{}()
           , ._maximum_number_of_parallel_streams = 0
           }
        };
      off_t const _data_offset
        { RandomOffT
          { RandomOffT::Min {0}
          , RandomOffT::Max {util::cast<off_t> (_memory_size) - 1}
          }()
        };
      std::size_t const _data_size
        { RandomDataSize
          { RandomDataSize::Min {1}
          , RandomDataSize::Max
              {_memory_size - util::cast<std::size_t> (_data_offset)}
          }()
        };
    };
  }

  TEST_F ( IOV_BackendWithNoCommunicationBufferR
         , direct_communication_works_without_any_communication_buffers
         )
  {
    auto original_data {_iov_backend.allocate (_data_size)};
    generate_random_data (*original_data);

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_WRITE
      ( *original_data
      , _data_offset
      );

    auto retrieved_data {_iov_backend.allocate (_data_size)};

    MCS_TEST_IOV_BACKEND_ASSERT_SUCCESSFUL_READ
      ( *retrieved_data
      , _data_offset
      );

    ASSERT_THAT (*original_data, ::testing::ElementsAreArray (*retrieved_data));
  }
}
