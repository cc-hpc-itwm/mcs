// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/STD/tuple.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>
#include <utility>

namespace mcs::rpc
{
  template<typename... Signals>
    ScopedIOContext::ScopedIOContext
      ( NumberOfThreads number_of_threads
      , Signals&&... signals
      )
        : asio::io_context {util::cast<int> (number_of_threads.value)}
        , _number_of_threads {number_of_threads}
        , _signals {*this, std::forward<Signals> (signals)...}
  {
    _signals.async_wait
      ( [this] (auto, auto) noexcept
        {
          stop();
        }
      );
  }

  constexpr ScopedIOContext::NumberOfThreads::NumberOfThreads
    ( unsigned int initial_value
    )
      : value {initial_value}
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
    constexpr auto formatter<mcs::rpc::ScopedIOContext::NumberOfThreads>::parse
      ( ParseContext& context
      )
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::rpc::ScopedIOContext::NumberOfThreads>::format
      ( mcs::rpc::ScopedIOContext::NumberOfThreads const& number_of_threads
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "ScopedIOContext::NumberOfThreads {}"
      , std::make_tuple (number_of_threads.value)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<rpc::ScopedIOContext::NumberOfThreads>::read
      ( State<Char>& state
      ) -> rpc::ScopedIOContext::NumberOfThreads
  {
    prefix (state, "ScopedIOContext::NumberOfThreads");
    auto [value] {parse<std::tuple<unsigned int>> (state)};

    return rpc::ScopedIOContext::NumberOfThreads
      { value
      };
  }
}
