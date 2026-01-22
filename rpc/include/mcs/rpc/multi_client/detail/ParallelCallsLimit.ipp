// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/uint.hpp>
#include <tuple>

namespace mcs::rpc::multi_client
{
  constexpr ParallelCalls::AtMost::AtMost (unsigned int _value)
    : value {_value}
  {
    if (value == 0)
    {
      throw Error::MustBePositive{};
    }
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::rpc::multi_client::ParallelCallsLimit>::parse
      (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::rpc::multi_client::ParallelCallsLimit>::format
      ( mcs::rpc::multi_client::ParallelCallsLimit const& parallel_calls_limit
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return std::visit
      ( mcs::util::overloaded
        ( [&] (mcs::rpc::multi_client::ParallelCalls::Unlimited)
          {
            return fmt::format_to (ctx.out(), "ParallelCalls::Unlimited");
          }
        , [&] (mcs::rpc::multi_client::ParallelCalls::AtMost at_most)
          {
            return fmt::format_to
              ( ctx.out()
              , "ParallelCalls::AtMost {}"
              , std::make_tuple (at_most.value)
              );
          }
        )
      , parallel_calls_limit
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<rpc::multi_client::ParallelCallsLimit>::read
      ( State<Char>& state
      ) -> rpc::multi_client::ParallelCallsLimit
  {
    prefix (state, "ParallelCalls::");

    if (maybe_prefix (state, "Unlimited"))
    {
      return rpc::multi_client::ParallelCalls::Unlimited{};
    }

    if (maybe_prefix (state, "AtMost"))
    {
      auto [value] {parse<std::tuple<unsigned int>> (state)};

      return rpc::multi_client::ParallelCalls::AtMost {value};
    }

    throw state.error ("Expected: 'Unlimited' | 'AtMost (UINT)'");
  }
}
