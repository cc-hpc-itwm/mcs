// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <asio/thread_pool.hpp>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <optional>

namespace mcs::rpc
{
  struct [[nodiscard]] ScopedIOContext : public asio::io_context
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
          auto operator= (MustBePositive&&) noexcept -> MustBePositive& = default;

        private:
          friend ScopedIOContext;

          MustBePositive() noexcept;
        };
      };

      unsigned int value {1u};
    };

    template<typename... Signals>
      explicit ScopedIOContext
        ( NumberOfThreads
        , Signals&&...
        );

    auto start() -> void;
    auto join() noexcept -> void;

    ~ScopedIOContext();
    ScopedIOContext (ScopedIOContext const&) = delete;
    ScopedIOContext (ScopedIOContext&&) = delete;
    auto operator= (ScopedIOContext const&) -> ScopedIOContext& = delete;
    auto operator= (ScopedIOContext&&) -> ScopedIOContext& = delete;

  private:
    NumberOfThreads _number_of_threads;
    asio::signal_set _signals;
    std::optional<asio::thread_pool> _threads;
    bool _started {false};
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::rpc::ScopedIOContext::NumberOfThreads>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::ScopedIOContext::NumberOfThreads const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<rpc::ScopedIOContext::NumberOfThreads>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> rpc::ScopedIOContext::NumberOfThreads
        ;
  };
}

#include "detail/ScopedIOContext.ipp"
