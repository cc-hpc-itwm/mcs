// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <memory>
#include <utility>

namespace mcs::rpc::detail
{
  template<typename Ret>
    constexpr ResultHolder::ResultHolder
      ( CallID call_id
      , ResultOrError<Ret> result_or_error
      )
        : archive
          { [ call_id
            , result_or_error = std::make_shared<ResultOrError<Ret>>
                (std::move (result_or_error))
            ]
            () mutable
            {
              return serialization::OArchive {call_id, *result_or_error};
            }
          }
  {}
}
