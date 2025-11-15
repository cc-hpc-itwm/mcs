// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/define.hpp>

namespace mcs::serialization
{
  template< typename Tracer
          , core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      )
  {
    using Description
      = core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      ;

    return save (oa, static_cast<typename Description::Base> (description));
  }

  template< typename Tracer
          , core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      )
  {
    using Description
      = core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      ;

    return Description {load<typename Description::Base> (ia)};
  }
}

namespace fmt
{
  template< typename Tracer
          , mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      )
  {
    return context.begin();
  }

  template< typename Tracer
          , mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( description
      , context
      , mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      )
  {
    using Description
      = mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
      ;

    return fmt::format_to
      ( context.out()
      , "{}"
      , static_cast<typename Description::Base> (description)
      );
  }
}
