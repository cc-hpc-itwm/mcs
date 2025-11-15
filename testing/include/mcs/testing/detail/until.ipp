// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::testing
{
  template<typename Pred, typename Fun>
    auto until (Pred pred, Fun fun) -> decltype (fun())
  {
    if (auto x {fun()}; pred (x))
    {
      return x;
    }
    else
    {
      return until (std::forward<Pred> (pred), std::forward<Fun> (fun));
    }
  }
}
