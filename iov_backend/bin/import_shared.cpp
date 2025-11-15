// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <list>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/control/command/file/Read.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/multi_client/call_and_collect.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/Timer.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <utility>
#include <variant>

namespace
{
  template<mcs::rpc::is_protocol Protocol>
    using StoragesClient = mcs::core::control::Client
       < Protocol
       , mcs::rpc::access_policy::Exclusive
       , mcs::iov_backend::SupportedStorageImplementations
       >
    ;

  // \todo use HeterogeneousMap
  using AnyStoragesClient = std::variant
    < StoragesClient<asio::ip::tcp>
    , StoragesClient<asio::local::stream_protocol>
    >
    ;
}

namespace
{
  struct Error
  {
    mcs::iov_backend::Location _location;
    std::exception_ptr _error;
  };
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (Error);
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, Error)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (error, context, Error)
  {
    return fmt::format_to
      ( context.out()
      , "{}@{}: {}"
      , error._location._range
      , error._location._storages_provider
      , error._error
      );
  }
}

namespace
{
  auto import_shared_main (mcs::util::Args args) -> int
  {
    auto timer_setup {mcs::util::Timer{}};

    if (args.size() != 6)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " provider"                       // 1
                        " collection_id"                  // 2
                        " collection_range"               // 3
                        " file_name"                      // 4
                        " file_range"                     // 5
                      , args[0]
                      )
        };
    }

    // \todo use multiple io_context threads!?
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    auto const collection_id {mcs::iov_backend::collection::ID {args[2]}};
    auto const collection_range
      { mcs::util::read::read<mcs::core::memory::Range> (args[3])
      };

    auto const file_name {std::filesystem::path {args[4]}};
    auto const file_range
      { mcs::util::read::read<mcs::core::memory::Range> (args[5])
      };

    if (size (collection_range) != size (file_range))
    {
      throw std::invalid_argument
        { fmt::format ( "Size mismatch: collection range {} has size {}"
                        " and file range {} has size {}"
                      , collection_range
                      , size (collection_range)
                      , file_range
                      , size (file_range)
                      )
        };
    }

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
      , [&]<mcs::util::ASIO::is_protocol ProviderProtocol>
          ( mcs::util::ASIO::Connectable<ProviderProtocol> provider_connectable
          )
        {
          auto const client
            { mcs::iov_backend::Client
                < ProviderProtocol
                , mcs::rpc::access_policy::Exclusive
                >
              { io_context
              , provider_connectable
              }
            };

          using FileRead = mcs::core::control::command::file::Read;

          auto errors {std::list<Error>{}};
          auto bytes_read {mcs::core::memory::make_size (0)};

          struct Collector
          {
            auto result ( mcs::iov_backend::Location const&
                        , FileRead::Response bytes_read
                        ) noexcept
            {
              *_bytes_read += bytes_read;
            }
            auto error ( mcs::iov_backend::Location const& location
                       , std::exception_ptr error
                       )
            {
              _errors->emplace_back (location, error);
            }

            mcs::core::memory::Size* _bytes_read;
            std::list<Error>* _errors;
          };
          auto collector
            { Collector { std::addressof (bytes_read)
                        , std::addressof (errors)
                        }
            };

          auto const time_setup {std::move (timer_setup).stop()};

          auto timer_transfer {mcs::util::Timer{}};

          mcs::rpc::multi_client::call_and_collect<FileRead>
            ( [&] (auto const& location)
              {
                auto const shifted_begin
                  { [&] (auto const& location_range)
                    {
                      if (begin (file_range) > begin (collection_range))
                      {
                        // collection_range    [..............)
                        // file_range          :   [..............)
                        //                     :   :
                        // delta               |>>>|
                        //
                        return begin (location_range)
                          + (begin (file_range) - begin (collection_range))
                          ;
                      }
                      else
                      {
                        // collection_range        [..............)
                        // file_range          [..............)
                        //                     :   :
                        // delta               |<<<|
                        //
                        return begin (location_range)
                          - (begin (collection_range) - begin (file_range))
                          ;
                      }
                    }
                  };

                return FileRead
                  { location._address.storage_id
                  , location._parameter_file_read
                  , location._address.segment_id
                  , location._address.offset
                  , file_name
                  , mcs::core::memory::make_range
                    ( shifted_begin (location._range)
                    , size (location._range)
                    )
                  };
              }
            , [&] (auto const& location)
              {
                return mcs::util::ASIO::run
                  ( location._storages_provider
                  , [&]<mcs::util::ASIO::is_protocol Protocol>
                      ( mcs::util::ASIO::Connectable<Protocol> provider
                      ) -> AnyStoragesClient
                    {
                      return StoragesClient<Protocol> {io_context, provider};
                    }
                  );
              }
            , collector
            , client.locations (collection_id, collection_range)
              // \todo parameter for the ParallelCallsLimit
            , mcs::rpc::multi_client::ParallelCalls::Unlimited{}
            );

          if (!errors.empty())
          {
            throw std::runtime_error {fmt::format ("Errors: {}", errors)};
          }

          auto const time_transfer {std::move (timer_transfer).stop()};

          fmt::print
            ( stderr
            , "setup: {}, transferred {} bytes in {} -> {} MiB/sec\n"
            , time_setup
            , bytes_read
            , time_transfer
            , time_transfer
              . per_second (size_cast<std::size_t> (bytes_read)) / (1L << 20L)
            );

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, import_shared_main);
}
