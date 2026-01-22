// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <asio/thread_pool.hpp>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::rpc
{
  // io_context that stops when one of the specified signals arrives
  // uses a thread pool for multi-threaded execution. At creation the
  // io_context's run() method is called in each thread of the
  // pool. Upon destruction the io_context's stop() method is called
  // and the threads are joined.
  //
  // EXAMPLE:
  //   auto io_context
  //     { ScopedRunningIOContext
  //         {ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
  //     };
  //
  struct [[nodiscard]] ScopedRunningIOContext : public asio::io_context
  {
    struct NumberOfThreads
    {
      constexpr explicit NumberOfThreads (unsigned int);

      struct Error
      {
        struct MustBePositive : public mcs::Error
        {
        public:
          ~MustBePositive() override;
          MustBePositive (MustBePositive const&) = default;
          MustBePositive (MustBePositive&&) noexcept = default;
          auto operator= (MustBePositive const&) -> MustBePositive& = default;
          auto operator= (MustBePositive&&) noexcept  -> MustBePositive& = default;

        private:
          friend ScopedRunningIOContext;

          MustBePositive() noexcept;
        };
      };

      unsigned int value {1u};
    };

    template<typename... Signals>
      explicit ScopedRunningIOContext
        ( NumberOfThreads
        , Signals&&...
        );

    // For servers that want to join without calling stop()
    // explicitly.
    //
    auto join() noexcept -> void;

    ~ScopedRunningIOContext();
    ScopedRunningIOContext (ScopedRunningIOContext const&) = delete;
    ScopedRunningIOContext (ScopedRunningIOContext&&) = delete;
    auto operator= (ScopedRunningIOContext const&) -> ScopedRunningIOContext& = delete;
    auto operator= (ScopedRunningIOContext&&) -> ScopedRunningIOContext& = delete;

  private:
    NumberOfThreads _number_of_threads;
    asio::thread_pool _threads;
    asio::signal_set _signals;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::rpc::ScopedRunningIOContext::NumberOfThreads>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::ScopedRunningIOContext::NumberOfThreads const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<rpc::ScopedRunningIOContext::NumberOfThreads>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> rpc::ScopedRunningIOContext::NumberOfThreads
        ;
  };
}

#include "detail/ScopedRunningIOContext.ipp"
