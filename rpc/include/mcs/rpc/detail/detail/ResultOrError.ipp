// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::rpc::detail
{
  template<typename T>
    Result<T>::Result (decltype (value) _value)
      : value {std::move (_value)}
  {}

  template<typename T>
    Result<T>::Result (serialization::IArchive& ia)
      : Result {load<decltype (value)> (ia)}
  {}

  template<typename T>
    auto Result<T>::save (serialization::OArchive& oa) const
      -> serialization::OArchive&
  {
    return serialization::save (oa, value);
  }

  template<typename T>
    auto make_result (T value) -> ResultOrError<T>
  {
    return ResultOrError<T> {Result<T> {std::move (value)}};
  }

  template<typename T>
    auto make_error (std::string reason) -> ResultOrError<T>
  {
    return ResultOrError<T> {Error {reason}};
  }
}
