// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/define.hpp>

namespace mcs::serialization
{
  template<typename Tracer, core::storage::is_implementation Storage>
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , create
      , core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, create, _parameter_tracer_create);
    MCS_SERIALIZATION_SAVE_FIELD (oa, create, _parameter_storage_create);

    return oa;
  }

  template<typename Tracer, core::storage::is_implementation Storage>
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      )
  {
    using Create = core::storage::implementation::trace::parameter
      ::Create<Tracer, Storage>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _parameter_tracer_create, Create);
    MCS_SERIALIZATION_LOAD_FIELD (ia, _parameter_storage_create, Create);

    return Create {_parameter_tracer_create, _parameter_storage_create};
  }
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      )
  {
    return context.begin();
  }

  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( create
      , context
      , mcs::core::storage::implementation::trace::parameter
          ::Create<Tracer, Storage>
      )
  {
    return fmt::format_to
      ( context.out()
      , "TraceStorage ({}, {})"
      , create._parameter_tracer_create
      , create._parameter_storage_create
      );
  }
}
