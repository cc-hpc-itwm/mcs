// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <iov/iov.hpp>
#include <iov/meta.hpp>
#include <iterator>
#include <mcs/IOV_Database.hpp>
#include <mcs/IOV_Meta.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/iov_backend/Provider.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/detail/Queue.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/iov_backend/IOV_Backend.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/write_file.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <numeric>
#include <span>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mcs::iov_backend
{
  namespace
  {
    auto be_patient()
    {
      return std::chrono::steady_clock::now() + std::chrono::seconds (5);
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

      core::Storages<SupportedStorageImplementations> _storages;

      rpc::ScopedRunningIOContext _io_context_storages_provider
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        };

      core::control::Provider
        < asio::local::stream_protocol
        , SupportedStorageImplementations
        > _storages_provider
            { _io_context_storages_provider
            , asio::local::stream_protocol::endpoint{}
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
        };

      using TransportProvider
        = core::transport::implementation::ASIO::Provider
            < asio::local::stream_protocol
            , SupportedStorageImplementations
            >
        ;
      TransportProvider _transport_provider
        { _io_context_transport_provider
        , asio::local::stream_protocol::endpoint{}
        , std::addressof (_storages)
        };

      storage::ID _storage_id;
    };
  }

  namespace
  {
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
        };

      rpc::ScopedRunningIOContext _io_context_provider_storages_clients
        { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
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

      RunningStorage<core::storage::implementation::Heap> _storage
        { core::memory::make_size (_memory_size)
        , _provider.local_endpoint()
        };

      IOV_Database _database;
      IOV_DBRef _database_ref {std::addressof (_database)};

      iov::meta::Name _workspace_name {"ws"};
      iov::meta::Name _collection_name {"cl"};

      iov::meta::MetaData _meta_data_workspace
        { iov_backend::invoke_and_throw_on_unexpected
          ( iov::meta::MetaData::create_workspace
          , fmt::format ( "Could not create workspace '{}'"
                        , _workspace_name
                        )
          , _workspace_name
          , std::addressof (_database_ref)
          )
        };
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
          , fmt::format ("Could not create collection '{}'", _collection_name)
          , _collection_name
          , _workspace_name
          , std::addressof (_database_ref)
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

  // multiple producers and multiple consumers:
  //
  // memory: divide it into virtual "slots"
  //
  // task: has an id, has data for one slot, there are more tasks than slots
  //
  // producers: as long as there are tasks to create:
  //   - get a free slot
  //   - do produce the data
  //   - write it into the memory
  //   - signal the task is ready, e.g. push into queue
  //
  // consumers: as long as there are tasks to consume:
  //   - get a task id and its slot
  //   - read the data from the memory
  //   - verify the data is correct
  //   - signal the slot being free, e.g. push into queue
  //
  // initial:
  //   - signal all slots are free, e.g. push into queue
  //
  // dependencies:
  //
  //*  ( next_task_id )          ( free slot )          ( tasks done )    */
  //*    atomic<id>                queue<slot>            atomic<count>   */
  //*               \             /         ^              ^              */
  //*                \           /           \            /               */
  //*                 v         v             \          /                */
  //*                 [ produce ]              [ consume ]                */
  //*                 ^         \              ^         ^                */
  //*                /           \            /           \               */
  //*               /             v          /             \              */
  //*          ( num_tasks )     ( task ready )          ( num_tasks )    */
  //                              queue<id, slot>
  //
  // concurrency:
  //   - use a single Heap memory in order to have as much concurrent
  //     accesses as possible
  //   - let all threads wait at a barrier for all of them to be ready
  //
  TEST (IOV_Backend, multiple_producers_and_multiple_consumers_are_allowed)
  {
    auto const memory_size {4ul << 30ul};
    auto const slot_size {16ul << 20ul};

    ASSERT_GE (slot_size, sizeof (int));

    auto running_iov_backend {RunningIOVBackend {memory_size}};
    auto iov_backend
      { testing::iov_backend::IOV_Backend
        {  Parameter {running_iov_backend.provider()}
        << Parameter::NumberOfThreads
           { ._transport_clients
               = rpc::ScopedRunningIOContext::NumberOfThreads {4u}
           }
        << Parameter::IndirectCommunication
           { ._number_of_buffers = 0
           , ._maximum_transfer_size = core::memory::make_size (0)
           , ._acquire_buffer_timeout_in_milliseconds = 0
           , ._maximum_number_of_parallel_streams = 0
           }
        << Parameter::DirectCommunication
           { ._maximum_transfer_size = core::memory::make_size (slot_size)
           , ._maximum_number_of_parallel_streams = 4
           }
        }
      };

    // \todo close the collection
    {
      auto const collection_open_result
        { iov_backend.collection_open
          ( iov::CollectionOpenParam
            { .name = iov::meta::Name {"ignored_by_backend_ops"}
            , .meta_data = running_iov_backend.meta_data()
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
                    , .max = iov::size::limit::Value {memory_size}
                    }
                }
              }
            , .workspace = *running_iov_backend.workspace()
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
    }

    auto const collection_delete
      { nonstd::make_scope_exit_that_dies_on_exception
          ( "collection_delete"
          , [&]
            {
              auto const collection_delete_result
                { iov_backend.collection_delete
                  ( iov::DeleteParam
                    { .name = iov::meta::Name {"ignored_by_backend_ops"}
                    , .meta_data = running_iov_backend.meta_data()
                    }
                  ).get()
                };

              if (!collection_delete_result.result)
              {
                fmt::print ( "collection_delete: {}"
                           , collection_delete_result.result.error().reason()
                           );
              }
            }
          )
      };

    auto const number_of_slots {memory_size / slot_size};
    auto const number_of_tasks {12ul * number_of_slots};

    struct Slot
    {
      unsigned long _id;
    };
    struct Task
    {
      unsigned long _id;
    };
    struct ReadyTask
    {
      Slot _slot;
      Task _task;
    };

    auto const produce_data
      { [&] (auto const& buffer, Slot slot, Task task)
        {
          auto* elements {util::cast<unsigned long*> (buffer->data())};
          elements[0] = task._id;
          auto const write_request_data
            { iov_backend.write
                ( { running_iov_backend.collection()
                  , *buffer
                  , util::cast<off_t> (slot._id * slot_size)
                  }
                )
            . get()
            };
          if (!write_request_data.result)
          {
            throw std::runtime_error
              { fmt::format ( "Write: Error: {}"
                            , write_request_data.result.error().reason()
                            )
              };
          }
        }
      };

    auto const consume_data
      { [&] (auto const& buffer, ReadyTask ready_task)
        {
          auto const read_request_data
            { iov_backend.read
                ( { running_iov_backend.collection()
                  , *buffer
                  , util::cast<off_t> (ready_task._slot._id * slot_size)
                  }
                )
            . get()
            };
          if (!read_request_data.result)
          {
            throw std::runtime_error
              { fmt::format ( "Read: Error: {}"
                            , read_request_data.result.error().reason()
                            )
              };
          }
          auto const* elements
            { util::cast<unsigned long const*> (buffer->data())
            };
          if (elements[0] != ready_task._task._id)
          {
            throw std::runtime_error
              { fmt::format ( "Read: Wrong data: {} != {}"
                            , elements[0]
                            , ready_task._task._id
                            )
              };
          }
        }
      };

    auto free_slots {iov_backend::detail::Queue<Slot>{}};
    auto ready_tasks {iov_backend::detail::Queue<ReadyTask>{}};

    auto tasks_produced {std::atomic<unsigned long> {0ul}};
    auto tasks_consumed {std::atomic<unsigned long> {0ul}};

    auto producers {std::vector<std::future<int>>{}};
    auto consumers {std::vector<std::future<int>>{}};

    auto const number_of_threads {std::thread::hardware_concurrency()};
    auto barrier {std::barrier {2 * number_of_threads + 1}};

    for (auto i {0ul}; i != number_of_threads; ++i)
    {
      producers.emplace_back
        ( std::async
          ( std::launch::async
          , [&]
            {
              auto const buffer {iov_backend.allocate (slot_size)};

              barrier.arrive_and_wait();

              auto _produced {0};

              for ( auto task {tasks_produced.fetch_add (1)}
                  ; task < number_of_tasks
                  ; task = tasks_produced.fetch_add (1)
                  )
              {
                auto const slot
                  { free_slots.get
                      ( decltype (free_slots)::InterruptionContext{}
                      , be_patient()
                      )
                  };

                produce_data (buffer, slot, Task {task});

                ++_produced;

                ready_tasks.push (slot, Task {task});
              }

              return _produced;
            }
          )
        );
    }

    for (auto i {0u}; i != number_of_threads; ++i)
    {
      consumers.emplace_back
        ( std::async
          ( std::launch::async
          , [&]
            {
              auto const buffer {iov_backend.allocate (slot_size)};

              barrier.arrive_and_wait();

              auto _consumed {0};

              for ( auto task {tasks_consumed.fetch_add (1)}
                  ; task < number_of_tasks
                  ; task = tasks_consumed.fetch_add (1)
                  )
              {
                auto const ready_task
                  { ready_tasks.get
                      ( decltype (ready_tasks)::InterruptionContext{}
                      , be_patient()
                      )
                  };

                consume_data (buffer, ready_task);

                ++_consumed;

                free_slots.push (ready_task._slot);
              }

              return _consumed;
            }
          )
        );
    }

    // start by signalling all slots as free...
    for ( auto slot_id {0}
        ; std::cmp_not_equal (slot_id, number_of_slots)
        ; ++slot_id
        )
    {
      free_slots.push (slot_id);
    }

    using Clock = std::chrono::steady_clock;
    auto const start {Clock::now()};

    // ...release all threads...
    barrier.arrive_and_wait();

    // ...and wait for all tasks to be processed
    auto const produced
      { std::accumulate
          ( std::begin (producers), std::end (producers)
          , 0
          , [] (auto sum, auto& producer)
            {
              return sum + producer.get();
            }
          )
      };
    auto const consumed
      { std::accumulate
          ( std::begin (consumers), std::end (consumers)
          , 0
          , [] (auto sum, auto& consumer)
            {
              return sum + consumer.get();
            }
          )
      };
    auto const end {Clock::now()};

    ASSERT_EQ (produced, number_of_tasks);
    ASSERT_EQ (consumed, number_of_tasks);

    auto const duration
      { std::chrono::duration_cast<std::chrono::microseconds> (end - start)
      };

    fmt::print
      ( "memory_size {} slot_size {} #task {} #thread {} time {} bytes {} -> {} MB/s\n"
      , memory_size
      , slot_size
      , number_of_tasks
      , number_of_threads
      , duration
      ,  2ul * slot_size * number_of_tasks
      , (2ul * slot_size * number_of_tasks)
        / util::cast<unsigned long> (duration.count())
      );
  }
}
