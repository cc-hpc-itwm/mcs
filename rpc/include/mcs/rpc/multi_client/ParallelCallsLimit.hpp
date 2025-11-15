// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
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
          MCS_ERROR_COPY_MOVE_DEFAULT (MustBePositive);

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
  template<> MCS_UTIL_FMT_DECLARE (mcs::rpc::multi_client::ParallelCallsLimit);
}

namespace mcs::util::read
{
  template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (rpc::multi_client::ParallelCallsLimit);
}

#include "detail/ParallelCallsLimit.ipp"
