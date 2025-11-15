// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "detail/Progress.hpp"
#include "detail/error.hpp"
#include "detail/queue_get.hpp"
#include "workspace_name.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <functional>
#include <future>
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
#include <mcs/util/divru.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/select.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/pwrite.hpp>
#include <mcs/util/touch.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

MCS_IOV_BACKEND_BIN_MAKE_ERROR (ProducerError);
MCS_IOV_BACKEND_BIN_MAKE_ERROR (ReaderError);
MCS_IOV_BACKEND_BIN_MAKE_ERROR (WriterError);

namespace
{
  auto export_main (mcs::util::Args args) -> int
  {
    auto timer_setup {mcs::util::Timer{}};

    if (args.size() != 11)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {}"
                        " configuration_file"             // 1
                        " database_file"                  // 2
                        " collection_id"                  // 3
                        " collection_range"               // 4
                        " file_name"                      // 5
                        " file_range"                     // 6
                        " number_of_reader_threads"       // 7
                        " number_of_writer_threads"       // 8
                        " buffer_size"                    // 9
                        " number_of_buffers"              // 10
                      , args[0]
                      )
        };
    }

    auto const configuration_file {mcs::util::ExistingPath {args[1]}};
    auto const database_file {mcs::util::ExistingPath {args[2]}};
    auto const collection_id {mcs::iov_backend::collection::ID {args[3]}};
    auto const collection_range
      { mcs::util::read::read<mcs::core::memory::Range> (args[4])
      };

    auto const file_name {mcs::util::touch (args[5])};
    auto const file_range
      { mcs::util::read::read<mcs::core::memory::Range> (args[6])
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
    auto const size {mcs::core::memory::size (file_range)};

    auto const number_of_reader_threads
      { mcs::util::read::read<std::size_t> (args[7])
      };
    auto const number_of_writer_threads
      { mcs::util::read::read<std::size_t> (args[8])
      };
    auto const buffer_size
      { std::min
        ( size
        , mcs::util::read::read<mcs::core::memory::Size> (args[9])
        )
      };
    auto const number_of_chunks
      { mcs::util::divru ( size_cast<std::size_t> (size)
                    , size_cast<std::size_t> (buffer_size)
                    )
      };
    auto const number_of_buffers
      { std::invoke
        ( [&]
          {
            return std::min
              ( mcs::util::read::read<std::size_t> (args[10])
              , number_of_chunks
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
    // - 'Available' to be used
    // - 'Assigned' to be filled with data from the input
    // - 'Filled' with data to be written to a certain offset in the output.
    //
    // There are three independent kind of executions:
    //
    // - A single producer that assigns offset to available buffers.
    //   The producer creates at most number_of_buffer many buffers.
    //
    // - A number of concurrent 'reader's which read data from the
    //   input into assigned buffers.
    //
    // - A number of concurrent 'writer's which write data into the
    //   output collection.
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

    struct Offset
    {
      mcs::core::memory::Offset file;
      mcs::core::memory::Offset collection;

      auto operator+= (mcs::core::memory::Size n) -> Offset&
      {
        file += n;
        collection += n;

        return *this;
      }
    };

    struct AssignedBuffer
    {
      std::span<std::byte> buffer;
      Offset offset;
      mcs::core::memory::Size transfer_size;
    };

    auto assigned_buffers {mcs::iov_backend::detail::Queue<AssignedBuffer>{}};
    auto assigned_buffers_interruption_context
      { decltype (assigned_buffers)::InterruptionContext{}
      };

    struct FilledBuffer
    {
      std::span<std::byte> buffer;
      Offset offset;
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
          , std::addressof (assigned_buffers)
          )
      };

    auto progress {Progress {size}};

    auto writers {std::vector<std::future<void>>{}};

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
                      auto const open_file {mcs::util::fopen (file_name, "r+b")};
                      auto const fd {mcs::util::syscall::fileno (open_file.get())};

                      while ( auto filled_buffer
                                { queue_get
                                    ( "Filled"
                                    , filled_buffers
                                    , filled_buffers_interruption_context
                                    )
                                }
                            )
                      {
                        auto const nbyte
                         { size_cast<std::size_t>
                             ( filled_buffer->transfer_size
                             )
                         };

                        auto const bytes_written
                          { mcs::util::syscall::pwrite
                            ( fd
                            , filled_buffer->buffer.data()
                            , nbyte
                            , make_off_t (filled_buffer->offset.file)
                            )
                          };

                        if (std::cmp_not_equal (bytes_written, nbyte))
                        {
                          throw std::runtime_error
                            { fmt::format
                                ( "Error when writing to offset {}"
                                   " : Wanted {} bytes, wrote {} bytes"
                                , filled_buffer->offset.file
                                , nbyte
                                , bytes_written
                                )
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
                       while ( auto const assigned_buffer
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
                                    , make_off_t
                                        ( assigned_buffer->offset.collection
                                        )
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

    auto const assign_chunk
      { [&] (auto chunk)
        {
          auto offset
            { Offset
              { begin (file_range)       + chunk * buffer_size
              , begin (collection_range) + chunk * buffer_size
              }
            };

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

          auto const transfer_size
            { std::min (buffer_size, end (file_range) - offset.file)
            };

          assigned_buffers.push (buffer, offset, transfer_size);
        }
      };

    auto const time_setup {std::move (timer_setup).stop()};

    auto timer_transfer {mcs::util::Timer{}};
    error_collector.collect_errors_as<ProducerError>
      ( [&]
        {
          // \todo determine number of stripes, even better: take
          // distribution of collection into account
          auto const number_of_stripes {decltype (number_of_chunks) {4}};
          auto const number_of_rounds
            {mcs::util::divru (number_of_chunks, number_of_stripes)};

          for ( auto round {decltype (number_of_chunks) {0}}
              ; round != number_of_rounds
              ; ++round
              )
          {
            for ( auto stripe {decltype (number_of_stripes) {0}}
                ; stripe != number_of_stripes
                ; ++stripe
                )
            {
              if ( auto const chunk {stripe * number_of_rounds + round}
                 ; chunk < number_of_chunks
                 )
              {
                assign_chunk (chunk);
              }
            }
          }

          progress.wait_done();

          assigned_buffers.interrupt (assigned_buffers_interruption_context);
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

    std::ranges::for_each
      ( readers
      , [&] (auto& reader)
        {
          reader.get();
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
      , "setup: {}, transferred {} bytes in {} -> {} MiB/sec\n"
      , time_setup
      , size
      , time_transfer
      , time_transfer.per_second (size_cast<std::size_t> (size)) / (1L << 20L)
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, export_main);
}
