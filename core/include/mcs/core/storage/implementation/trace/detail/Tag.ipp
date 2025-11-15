// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::trace::Tag<Tracer, Storage>
      )
  {
    return context.begin();
  }

  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( /* tag */
      , context
      , mcs::core::storage::implementation::trace::Tag<Tracer, Storage>
      )
  {
    return fmt::format
      ( context.out()
      , "Traced<{},{}>"
      , typename Tracer::Tag{}
      , typename Storage::Tag{}
      );
  }
}
