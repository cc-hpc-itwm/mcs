// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/Progress.hpp"
#include "detail/error.hpp"
#include "detail/queue_get.hpp"
#include "workspace_name.hpp"
#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <functional>
#include <future>
#include <ios>
#include <iostream>
#include <iov/meta.hpp>
#include <iterator>
#include <mcs/IOV_Backend.hpp>
#include <mcs/IOV_Database.hpp>
#include <mcs/IOV_Meta.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/iov_backend/detail/Queue.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/util/ExistingPath.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/Timer.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/divru.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/select.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

MCS_IOV_BACKEND_BIN_MAKE_ERROR (ReaderError);
MCS_IOV_BACKEND_BIN_MAKE_ERROR (WriterError);

namespace
{
  auto write_main (mcs::util::Args args) -> int
  {
    auto timer_setup {mcs::util::Timer{}};

    if (args.size() != 8)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: echo 'DATA' | {}"
                        " configuration_file"               // 1
                        " database_file"                    // 2
                        " collection_id"                    // 3
                        " range"                            // 4
                        " number_of_writer_threads"         // 5
                        " buffer_size"                      // 6
                        " number_of_buffers"                // 7
                      , args[0]
                      )
        };
    }

    auto const configuration_file {mcs::util::ExistingPath {args[1]}};
    auto const database_file {mcs::util::ExistingPath {args[2]}};
    auto const collection_id {mcs::iov_backend::collection::ID {args[3]}};

    auto const range
      { mcs::util::read::read<mcs::core::memory::Range> (args[4])
      };
    auto const number_of_writer_threads
      { mcs::util::read::read<std::size_t> (args[5])
      };
    auto const buffer_size
      { std::min
        ( size (range)
        , mcs::util::read::read<mcs::core::memory::Size> (args[6])
        )
      };
    auto const number_of_buffers
      { std::invoke
        ( [&]
          {
            return std::min
              ( mcs::util::read::read<std::size_t> (args[7])
              , mcs::util::divru ( size_cast<std::size_t> (size (range))
                            , size_cast<std::size_t> (buffer_size)
                            )
              );
          }
        )
      };

    auto database {mcs::make_scoped_iov_database (*database_file)};
    auto database_ref {mcs::IOV_DBRef {std::addressof (*database)}};

    auto iov_backend {mcs::IOV_Backend {configuration_file}};
    auto iov_backend_context {iov_backend.context (std::addressof (*database))};

    auto workspace
      { iov::Workspace
        { mcs::iov_backend::invoke_and_throw_on_unexpected
            ( iov::meta::MetaData::load_workspace
            , fmt::format
                ( "IOV: Could not load workspace '{}' in database '{}'"
                , mcs::iov_backend::workspace_name()
                , *database_file
                )
            , mcs::iov_backend::workspace_name()
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

    auto collection
      { iov::Collection
        { workspace
        , mcs::iov_backend::invoke_and_throw_on_unexpected
            ( iov::meta::MetaData::load_collection
            , fmt::format
                ( "IOV: Could not load collection '{}'"
                  " in workspace '{}' in database '{}'"
                , collection_id._uuid
                , mcs::iov_backend::workspace_name()
                , *database_file
                )
            , collection_id._uuid
            , mcs::iov_backend::workspace_name()
            , std::addressof (database_ref)
            )
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
        }
      };

    auto buffers {std::vector<mcs::IOV_Backend::Allocation>{}};

    // The program works with two queues of buffers:
    // - 'Available' to be used
    // - 'Filled' with data to be written to a certain offset.
    //
    // There are two independent kind of executions:
    //
    // - A single 'reader' that reads data from cin into available
    //   buffers. The reader creates at most number_of_buffer many
    //   buffers.
    //
    // - A number of concurrent 'writer'S which write data into the
    //   output collection.
    //
    // The "buffer flow" can be depicted by:
    //
    //        [ available ]
    //          v      ^
    //     (reader)  (writer)
    //          v      ^
    //         [ filled ]

    struct AvailableBuffer
    {
      std::span<std::byte> buffer;
    };

    auto available_buffers {mcs::iov_backend::detail::Queue<AvailableBuffer>{}};

    struct FilledBuffer
    {
      std::span<std::byte> buffer;
      mcs::core::memory::Offset offset;
      mcs::core::memory::Size transfer_size;
    };

    auto filled_buffers {mcs::iov_backend::detail::Queue<FilledBuffer>{}};
    auto filled_buffers_interruption_context
      { decltype (filled_buffers)::InterruptionContext{}
      };

    auto error_collector
      { make_error_collector
          ( std::addressof (filled_buffers)
          , std::addressof (available_buffers)
          )
      };

    auto writers {std::vector<std::future<void>>{}};

    auto progress {Progress {size (range)}};

    std::generate_n
      ( std::back_inserter (writers)
      , std::min (number_of_buffers, number_of_writer_threads)
      , [&]
        {
          return std::async
            ( std::launch::async
            , [&]()
              {
                error_collector.collect_errors_as<WriterError>
                  ( [&]
                    {
                      while ( auto filled_buffer
                                { queue_get
                                    ( "Filled"
                                    , filled_buffers
                                    , filled_buffers_interruption_context
                                    )
                                }
                            )
                      {
                        auto const write_request_data
                          { iov_backend
                          . write ( { std::addressof (collection)
                                    , mcs::util::select
                                      ( filled_buffer->buffer
                                      , 0
                                      , size_cast<std::size_t>
                                          ( filled_buffer->transfer_size
                                          )
                                      )
                                    , make_off_t (filled_buffer->offset)
                                    }
                                  )
                          . get()
                          };

                        if (!write_request_data.result)
                        {
                          throw std::runtime_error
                            { write_request_data.result.error().reason()
                            };
                        }

                        available_buffers.push (filled_buffer->buffer);

                        progress.bytes_transferred
                          ( filled_buffer->transfer_size
                          );
                      }
                    }
                  );
              }
            );
        }
      );
    auto const time_setup {std::move (timer_setup).stop()};

    auto timer_transfer {mcs::util::Timer{}};
    error_collector.collect_errors_as<ReaderError>
      ( [&]
        {
          auto to_transfer {size (range)};
          auto offset {begin (range)};

          while (offset < end (range))
          {
            auto const buffer
              { std::invoke
                ( [&]
                  {
                    if (buffers.size() < number_of_buffers)
                    {
                      return *buffers.emplace_back
                        ( iov_backend.allocate
                            ( size_cast<std::size_t> (buffer_size)
                            )
                        );
                    }
                    else
                    {
                      return queue_get ("Available", available_buffers)->buffer;
                    }
                  }
                )
              };

            auto const transfer_size {std::min (buffer_size, to_transfer)};

            if ( std::cin.read
                   ( mcs::util::cast<char*> (buffer.data())
                   , mcs::util::cast<std::streamsize>
                       ( size_cast<std::size_t> (transfer_size)
                       )
                   ).fail()
               )
            {
              throw std::runtime_error {"Failure, broken pipe."};
            }

            filled_buffers.push (buffer, offset, transfer_size);

            to_transfer -= transfer_size;
            offset += transfer_size;
          }

          if (std::cin.peek() != std::char_traits<char>::eof())
          {
            fmt::print
              ( stderr
              , "Warning: Too much data provided"
                ": range '{}' has size '{}' and input contains more bytes.\n"
              , range
              , size (range)
              );
          }

          progress.wait_done();

          filled_buffers.interrupt (filled_buffers_interruption_context);
        }
      );

    std::ranges::for_each
      ( writers
      , [&] (auto& writer)
        {
          writer.get();
        }
      );

    if (!error_collector.empty())
    {
      throw std::runtime_error
        { fmt::format ("Errors: {}", error_collector.errors())
        };
    }

    auto const time_transfer {std::move (timer_transfer).stop()};

    fmt::print
      ( stderr
      , "setup: {}, written {} bytes in {} -> {} MiB/sec\n"
      , time_setup
      , size (range)
      , time_transfer
      , time_transfer
        . per_second (size_cast<std::size_t> (size (range))) / (1L << 20L)
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, write_main);
}
