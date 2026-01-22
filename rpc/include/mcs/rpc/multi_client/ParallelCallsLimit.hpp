// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <variant>

namespace mcs::rpc::multi_client
{
  struct ParallelCalls
  {
    struct Unlimited{};
    struct AtMost
    {
      constexpr explicit AtMost (unsigned int);

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
          friend ParallelCalls;

          MustBePositive() noexcept;
        };
      };

      unsigned int value {1u};
    };
  };

  using ParallelCallsLimit = std::variant
    < ParallelCalls::Unlimited
    , ParallelCalls::AtMost
    >;
}

namespace fmt
{
  template<>
    struct formatter<mcs::rpc::multi_client::ParallelCallsLimit>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::rpc::multi_client::ParallelCallsLimit const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<rpc::multi_client::ParallelCallsLimit>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> rpc::multi_client::ParallelCallsLimit
        ;
  };
}

#include "detail/ParallelCallsLimit.ipp"
