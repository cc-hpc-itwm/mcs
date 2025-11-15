// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/Progress.hpp"
#include "detail/error.hpp"
#include "detail/queue_get.hpp"
#include "workspace_name.hpp"
#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdio>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <functional>
#include <future>
#include <iostream>
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
#include <set>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

MCS_IOV_BACKEND_BIN_MAKE_ERROR (ProducerError);
MCS_IOV_BACKEND_BIN_MAKE_ERROR (ReaderError);
MCS_IOV_BACKEND_BIN_MAKE_ERROR (WriterError);

namespace
{
  auto cat_main (mcs::util::Args args) -> int
  {
    auto timer_setup {mcs::util::Timer{}};

    if (args.size() != 8)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " configuration_file"               // 1
                        " database_file"                    // 2
                        " collection_id"                    // 3
                        " range"                            // 4
                        " number_of_reader_threads"         // 5
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
    auto const number_of_reader_threads
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

    // The program works with three queues of buffers:
    // - 'Available' to be used.
    // - 'Assigned' an offset in the input data to be read into the buffer.
    // - 'Filled' with the data of the assigned offset.
    //
    // There are three independent kind of executions:
    //
    // - A single 'producer' that assigned offsets to available
    //   buffer. The producers creates at most number_of_buffer many
    //   buffers.
    //
    // - A number of concurrent 'reader's which read input data into
    //   assigned buffers.
    //
    // - A single 'writer' that writes data (in order) from filled
    //   buffers to cout.
    //
    // The "buffer flow" can be depicted by:
    //
    //     (producer)  <- [ available ] <-  (writer)
    //         v                               ^
    //    [ assigned ] ->   (reader)    -> [ filled ]

    struct AvailableBuffer
    {
      std::span<std::byte> buffer;
    };

    auto available_buffers {mcs::iov_backend::detail::Queue<AvailableBuffer>{}};

    struct AssignedBuffer
    {
      std::span<std::byte> buffer;
      mcs::core::memory::Offset offset;
      mcs::core::memory::Size transfer_size;
    };

    auto assigned_buffers {mcs::iov_backend::detail::Queue<AssignedBuffer>{}};
    auto assigned_buffers_interruption_context
      { decltype (assigned_buffers)::InterruptionContext{}
      };

    struct FilledBuffer
    {
      std::span<std::byte> buffer;
      mcs::core::memory::Offset offset;
      mcs::core::memory::Size transfer_size;

      [[nodiscard]] constexpr auto operator<=>
        ( FilledBuffer const& other
        ) const noexcept
      {
        return offset <=> other.offset;
      }
    };

    auto filled_buffers {mcs::iov_backend::detail::Queue<FilledBuffer>{}};
    auto filled_buffers_interruption_context
      { decltype (filled_buffers)::InterruptionContext{}
      };

    auto error_collector
      { make_error_collector
          ( std::addressof (filled_buffers)
          , std::addressof (assigned_buffers)
          , std::addressof (available_buffers)
          )
      };

    auto progress {Progress {size (range)}};

    auto writer
      { std::async
        ( std::launch::async
        , [&]()
          {
            error_collector.collect_errors_as<WriterError>
              ( [&]
                {
                  auto offset {begin (range)};
                  auto ready_buffers {std::set<FilledBuffer>{}};

                  while ( auto filled_buffer
                            { queue_get
                                ( "Filled"
                                , filled_buffers
                                , filled_buffers_interruption_context
                                )
                            }
                        )
                  {
                    ready_buffers.emplace (*filled_buffer);

                    while (  !ready_buffers.empty()
                          && std::begin (ready_buffers)->offset == offset
                          )
                    {
                      auto const ready_buffer {std::begin (ready_buffers)};
                      if ( std::cout.write
                             ( mcs::util::cast<char*>
                                 ( ready_buffer->buffer.data()
                                 )
                             , mcs::util::cast<std::streamsize>
                                 ( size_cast<std::size_t>
                                     ( ready_buffer->transfer_size
                                     )
                                 )
                             ).fail()
                         )
                      {
                        throw std::runtime_error {"Failure, broken pipe."};
                      }

                      available_buffers.push (ready_buffer->buffer);

                      progress.bytes_transferred (ready_buffer->transfer_size);

                      offset += ready_buffer->transfer_size;
                      ready_buffers.erase (ready_buffer);
                    }
                  }
                }
              );
          }
        )
      };

    auto readers {std::vector<std::future<void>>{}};

    std::generate_n
      ( std::back_inserter (readers)
      , std::min (number_of_buffers, number_of_reader_threads)
      , [&]
        {
          return std::async
             ( std::launch::async
             , [&]()
               {
                 error_collector.collect_errors_as<ReaderError>
                   ( [&]
                     {
                       while (auto const assigned_buffer
                                { queue_get
                                    ( "Assigned"
                                    , assigned_buffers
                                    , assigned_buffers_interruption_context
                                    )
                                }
                             )
                       {
                         auto const read_request_data
                           { iov_backend
                           . read ( { std::addressof (collection)
                                    , mcs::util::select
                                      ( assigned_buffer->buffer
                                      , 0
                                      , size_cast<std::size_t>
                                          ( assigned_buffer->transfer_size
                                          )
                                      )
                                    , make_off_t (assigned_buffer->offset)
                                    }
                                  )
                           . get()
                           };

                         if (!read_request_data.result)
                         {
                           throw std::runtime_error
                             { read_request_data.result.error().reason()
                             };
                         }

                         filled_buffers.push
                           ( assigned_buffer->buffer
                           , assigned_buffer->offset
                           , assigned_buffer->transfer_size
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
    error_collector.collect_errors_as<ProducerError>
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

            assigned_buffers.push (buffer, offset, transfer_size);

            to_transfer -= transfer_size;
            offset += transfer_size;
          }

          progress.wait_done();

          assigned_buffers.interrupt (assigned_buffers_interruption_context);
          filled_buffers.interrupt (filled_buffers_interruption_context);
        }
      );

    std::ranges::for_each
      ( readers
      , [&] (auto& reader)
        {
          reader.get();
        }
      );

    writer.get();

    if (!error_collector.empty())
    {
      throw std::runtime_error
        { fmt::format ("Errors: {}", error_collector.errors())
        };
    }

    auto const time_transfer {std::move (timer_transfer).stop()};

    fmt::print
      ( stderr
      , "setup: {} sec, read {} bytes in {} sec -> {} MiB/sec\n"
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
  return mcs::util::main (argc, argv, cat_main);
}
