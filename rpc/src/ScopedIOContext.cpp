// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/post.hpp>
#include <cstdio>
#include <exception>
#include <mcs/rpc/ScopedIOContext.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc
{
  auto ScopedIOContext::start() -> void
  {
    if (std::exchange (_started, true))
    {
      throw std::logic_error
        { "ScopedIOContext::start() may only be called once"
        };
    }

    try
    {
      _threads.emplace (_number_of_threads.value);

      for (auto worker {0u}; worker < _number_of_threads.value; ++worker)
      {
        asio::post
          ( *_threads
          , [this]() noexcept
            {
            RUN:
              try
              {
                asio::io_context::run();
              }
              catch (...)
              {
                util::FMT::print_noexcept
                  ( stderr
                  , "ScopedIOContext worker: {}\n"
                  , std::current_exception()
                  );

                // Continue to serve requests after exceptions.
                //
                goto RUN;
              }
            }
          );
      }
    }
    catch (...)
    {
      asio::io_context::stop();

      if (_threads)
      {
        _threads->stop();
        _threads->join();
        _threads.reset();
      }

      throw;
    }
  }

  auto ScopedIOContext::join() noexcept -> void
  {
    if (_threads)
    {
      _threads->join();
    }
  }

  ScopedIOContext::~ScopedIOContext()
  {
    asio::io_context::stop();

    join();
  }

  ScopedIOContext::NumberOfThreads::Error::MustBePositive::MustBePositive
    (
    ) noexcept
      : mcs::Error {"Number of threads must be positive."}
  {}
  ScopedIOContext::NumberOfThreads::Error::MustBePositive::~MustBePositive
    (
    ) = default
    ;
}
