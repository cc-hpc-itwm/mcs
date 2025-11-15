// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::testing
{
  // returns a value that fulfills the predicate
  //
  template<typename Pred, typename Fun>
    auto until (Pred, Fun fun) -> decltype (fun());
}

#include "detail/until.ipp"
