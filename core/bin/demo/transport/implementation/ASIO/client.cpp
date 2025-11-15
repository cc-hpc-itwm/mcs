// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <barrier>
#include <chrono>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <future>
#include <iterator>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  auto client_main (mcs::util::Args args) -> int
  {
    auto const usage
      { [&]
        {
          return fmt::format
            ( "usage: {} provider_path number_of_longs repetitions number_of_clients number_of_threads [get|put]"
            , args[0]
            )
            ;
        }
      };

    if (args.size() != 7)
    {
      throw std::invalid_argument {usage()};
    }

    auto const method {std::string (args[6])};
    if (method != "get" && method != "put")
    {
      throw std::invalid_argument {usage()};
    }
    auto const provider_path {std::filesystem::path (args[1])};

    auto const provider_connectable
      { mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (provider_path / "PROVIDER")
      };
    auto const source
      { mcs::util::read::from_file<mcs::core::transport::Address>
          (provider_path / "SOURCE")
      };

    auto const number_of_longs {mcs::util::read::read<unsigned int> (args[2])};
    auto const size_in_bytes {number_of_longs * sizeof (long)};
    auto const size {mcs::core::memory::make_size (size_in_bytes)};

    auto const repetitions {mcs::util::read::read<unsigned int> (args[3])};
    auto const number_of_clients
      { mcs::util::read::read<unsigned int> (args[4])
      };

    using Storage = mcs::core::storage::implementation::Heap;
    using SupportedStorageImplementations = mcs::util::type::List<Storage>;

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            { mcs::util::read::read<unsigned int> (args[5])
            }
        , SIGINT, SIGTERM
        }
      };

    using Duration = std::chrono::microseconds;
    using Clock = std::chrono::steady_clock;

    auto start_total {decltype (Clock::now()){}};

    auto clients {std::vector<std::future<Duration>>{}};
    auto barrier
      {std::barrier (number_of_clients, [&] { start_total = Clock::now(); })};

    std::generate_n
      ( std::back_inserter (clients)
      , number_of_clients
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]
              {
                auto report_barrier
                  { mcs::nonstd::make_scope_exit_that_dies_on_exception
                    ( "barrier::arrive"
                    , [&]
                      {
                        std::ignore = barrier.arrive();
                      }
                    )
                  };

                auto storages
                  { mcs::core::Storages<SupportedStorageImplementations>{}
                  };
                auto const storage
                  { mcs::core::make_unique_storage<Storage>
                      ( std::addressof (storages)
                      , Storage::Parameter::Create
                          { mcs::core::storage::MaxSize::Limit {size}
                          }
                      )
                  };
                auto const segment
                  { mcs::core::storage::make_unique_segment<Storage>
                      ( std::addressof (storages)
                      , storage->id()
                      , size
                      )
                  };

                auto const destination
                  { mcs::core::transport::Address
                    { storage->id()
                    , mcs::core::storage::make_parameter
                        ( typename Storage::Parameter::Chunk::Description{}
                        )
                    , segment->id()
                    , mcs::core::memory::make_offset (0)
                    }
                  };

                return mcs::util::ASIO::run
                  ( provider_connectable
                  , [&]<mcs::util::ASIO::is_protocol Protocol>
                      (mcs::util::ASIO::Connectable<Protocol> connectable)
                    {
                      auto const client
                        { mcs::core::transport::implementation::ASIO::Client
                               < Protocol
                               , mcs::rpc::access_policy::Exclusive
                               , SupportedStorageImplementations
                               >
                          { io_context
                          , connectable
                          , std::addressof (storages)
                          }
                        };

                      barrier.arrive_and_wait();
                      report_barrier.release();

                      auto const start {Clock::now()};
                      if (method == "get")
                      {
                        for (auto r {0u}; r != repetitions; ++r)
                        {
                          client.memory_get (destination, source, size).get();
                        }
                      }
                      else
                      {
                        for (auto r {0u}; r != repetitions; ++r)
                        {
                          client.memory_put (destination, source, size).get();
                        }
                      }
                      auto const end {Clock::now()};

                      return std::chrono::duration_cast<Duration> (end - start);
                    }
                  );
              }
            );
          }
      );

    auto durations {std::vector<Duration>{}};

    std::ranges::transform
      ( clients
      , std::back_inserter (durations)
      , [&] (auto& client) { return client.get(); }
      );

    auto const end_total {Clock::now()};

    auto const ticks
      { [] (auto d)
        {
          return mcs::util::cast<std::uintmax_t> (d.count());
        }
      };

    std::ranges::for_each
      ( durations
      , [&, c = 0] (auto duration) mutable
        {
          fmt::print
            ( "ASIO_memory_{}: client {}/{}: {} times {} bytes: {} -> {} per {} = {} MB/sec\n"
            , method
            , c++
            , number_of_clients
            , repetitions
            , size
            , duration
            , duration / repetitions
            , method
            , (repetitions * size_in_bytes) / ticks (duration)
            );
        }
      );

    auto const duration_total
      {std::chrono::duration_cast<Duration> (end_total - start_total)};

    auto const [min, max] {std::ranges::minmax_element (durations)};

    auto const bytes_total
      {   (repetitions * size_in_bytes * number_of_clients)
      };

    fmt::print
      ( "ASIO_memory_{}: clients {}: {} times {} bytes: total/min/max {}/{}/{} -> total/min/max {}/{}/{} MB/sec\n"
      , method
      , number_of_clients
      , repetitions
      , size
      , duration_total
      , *min
      , *max
      , bytes_total / ticks (duration_total)
      , bytes_total / ticks (*max)
      , bytes_total / ticks (*min)
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, client_main);
}
