// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  template< typename Tracer
          , core::storage::is_implementation Storage
          , core::chunk::is_access Access
          >
    requires (core::storage::trace::is_tracer<Tracer, Storage>)
    auto Implementation<core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>>::output
      ( OArchive& oa
      , core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access> const& description
      ) -> OArchive&
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
    auto Implementation<core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>
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
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template< typename Tracer
          , mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access>>::format
        ( mcs::core::storage::implementation::trace::chunk::Description<Tracer, Storage, Access> const& description
        , FormatContext& context
        ) const -> decltype (context.out())
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
