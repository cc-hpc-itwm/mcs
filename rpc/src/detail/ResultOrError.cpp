// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/detail/ResultOrError.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::rpc::detail
{
  Error::Error (decltype (reason) _reason)
    : reason {_reason}
  {}

  Error::Error (serialization::IArchive& ia)
    : reason {serialization::load<decltype (reason)> (ia)}
  {}

  auto Error::save (serialization::OArchive& oa) const
    -> serialization::OArchive&
  {
    return serialization::save (oa, reason);
  }

  auto make_result() -> ResultOrError<void>
  {
    return ResultOrError<void> {Result<void>{}};
  }
}
