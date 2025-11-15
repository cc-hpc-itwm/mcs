// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/detail/Completion.hpp>
#include <utility>

namespace mcs::rpc::detail
{
  auto Completion::operator() (std::exception_ptr rpc_error) -> void
  {
    _complete (rpc_error, Buffer{});
  }

  auto Completion::operator() (Buffer buffer) -> void
  {
    _complete (nullptr, std::move (buffer));
  }
}
