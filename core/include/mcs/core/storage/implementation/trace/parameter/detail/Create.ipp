// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  template<typename Tracer, core::storage::is_implementation Storage>
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    auto Implementation<core::storage::implementation::trace::parameter::Create<Tracer, Storage>>::output
      ( OArchive& oa
      , core::storage::implementation::trace::parameter::Create<Tracer, Storage> const& create
      ) -> OArchive&
  {
    save (oa, create._parameter_tracer_create);
    save (oa, create._parameter_storage_create);

    return oa;
  }

  template<typename Tracer, core::storage::is_implementation Storage>
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    auto Implementation<core::storage::implementation::trace::parameter::Create<Tracer, Storage>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::trace::parameter::Create<Tracer, Storage>
  {
    using Create = core::storage::implementation::trace::parameter
      ::Create<Tracer, Storage>
      ;

    auto _parameter_tracer_create {load<decltype (Create::_parameter_tracer_create)> (ia)};
    auto _parameter_storage_create {load<decltype (Create::_parameter_storage_create)> (ia)};

    return Create {_parameter_tracer_create, _parameter_storage_create};
  }
}

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::parameter::Create<Tracer, Storage>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::parameter::Create<Tracer, Storage>>::format
        ( mcs::core::storage::implementation::trace::parameter::Create<Tracer, Storage> const& create
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "TraceStorage ({}, {})"
      , create._parameter_tracer_create
      , create._parameter_storage_create
      );
  }
}
