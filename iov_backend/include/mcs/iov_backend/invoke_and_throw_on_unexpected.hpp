// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <string>
#include <type_traits>

namespace mcs::iov_backend
{
  // Invokes a function that returns an expected<R, iov::ErrorReason>
  // and returns the R or throws the error reason.
  //
  // EXAMPLE:
  //   invoke_and_throw_on_unexpected
  //     ( iov::meta::MetaData::load_collection
  //     , fmt::format
  //         ( "IOV: Could not load collection '{}'"
  //           " in workspace '{}' in database '{}'"
  //         , collection_name
  //         , mcs::iov_backend::workspace_name()
  //         , *database_file
  //         )
  //     , collection_name
  //     , mcs::iov_backend::workspace_name()
  //     , std::addressof (database_ref)
  //     )
  //
  template<typename Fun, typename... Args>
    auto invoke_and_throw_on_unexpected
      ( Fun&& fun
      , std::string error_description
      , Args&&... args
      ) -> typename std::invoke_result_t<Fun, Args&&...>::value_type
    ;
}

#include "detail/invoke_and_throw_on_unexpected.ipp"
