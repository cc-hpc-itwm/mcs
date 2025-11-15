// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/declare.hpp>
#include <mcs/util/read/uint.hpp>

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
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::rpc::multi_client::ParallelCallsLimit)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    (parallel_calls_limit, ctx, mcs::rpc::multi_client::ParallelCallsLimit)
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
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
    (state, rpc::multi_client::ParallelCallsLimit)
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
