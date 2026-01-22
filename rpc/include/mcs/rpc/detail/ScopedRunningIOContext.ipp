// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/post.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <utility>

namespace mcs::rpc
{
  template<typename... Signals>
    ScopedRunningIOContext::ScopedRunningIOContext
      ( NumberOfThreads number_of_threads
      , Signals&&... signals
      )
        : asio::io_context {util::cast<int> (number_of_threads.value)}
        , _number_of_threads {number_of_threads}
        , _threads {_number_of_threads.value}
        , _signals {*this, std::forward<Signals> (signals)...}
  {
    _signals.async_wait
      ( [&] (auto, auto)
        {
          stop();
        }
      );

    for (auto t {0u}; t < _number_of_threads.value; ++t)
    {
      asio::post
        ( _threads
        , [&]
          {
            asio::io_context::run();
          }
        );
    }
  }

  constexpr ScopedRunningIOContext::NumberOfThreads::NumberOfThreads
    ( unsigned int _value
    )
      : value {_value}
  {
    if (value == 0u)
    {
      throw Error::MustBePositive{};
    }
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::rpc::ScopedRunningIOContext::NumberOfThreads>::parse
      (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::rpc::ScopedRunningIOContext::NumberOfThreads>::format
      ( mcs::rpc::ScopedRunningIOContext::NumberOfThreads const& number_of_threads
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "ScopedRunningIOContext::NumberOfThreads {}"
      , number_of_threads.value
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<rpc::ScopedRunningIOContext::NumberOfThreads>::read
      ( State<Char>& state
      ) -> rpc::ScopedRunningIOContext::NumberOfThreads
  {
    prefix (state, "ScopedRunningIOContext::NumberOfThreads");

    return rpc::ScopedRunningIOContext::NumberOfThreads
      {parse<unsigned int> (state)};
  }
}
