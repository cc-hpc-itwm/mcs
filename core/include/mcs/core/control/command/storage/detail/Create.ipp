// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/type/List.hpp>
#include <tuple>
#include <utility>

namespace mcs::core::control::command::storage
{
  template< mcs::core::storage::is_implementation StorageImplementation
          , mcs::core::storage::is_implementation... StorageImplementations
          >
    auto create
      ( typename StorageImplementation::Parameter::Create parameter_create
      ) noexcept -> Create<StorageImplementations...>
  {
    return Create<StorageImplementations...>
      { util::type::List<StorageImplementations...>
          ::template id<StorageImplementation>()
      , mcs::core::storage::make_parameter (parameter_create)
      };
  }
}

namespace fmt
{
  template<mcs::core::storage::is_implementation... StorageImplementations>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::control::command::storage::Create<StorageImplementations...>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation... StorageImplementations>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::control::command::storage::Create<StorageImplementations...>>::format
        ( mcs::core::control::command::storage::Create<StorageImplementations...> const& create
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}"
      , std::make_tuple (create.implementation_id, create.storage_parameter)
      );
  }
}

namespace mcs::util::read
{
  template<core::storage::is_implementation... StorageImplementations>
    template<typename Char>
      auto Read<core::control::command::storage::Create<StorageImplementations...>>::read
        ( State<Char>& state
        ) -> core::control::command::storage::Create<StorageImplementations...>
  {
    namespace command = core::control::command;
    using Create = command::storage::Create<StorageImplementations...>;

    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::implementation_id)
               , decltype (Create::storage_parameter)
               >
             > (state)
      );
  }
}

namespace mcs::serialization
{
  template<core::storage::is_implementation... StorageImplementations>
    auto Implementation<core::control::command::storage::Create<StorageImplementations...>>::output
      ( OArchive& oa
      , core::control::command::storage::Create<StorageImplementations...> const& create
      ) -> OArchive&
  {
    save (oa, create.implementation_id);
    save (oa, create.storage_parameter);

    return oa;
  }

  template<core::storage::is_implementation... StorageImplementations>
    auto Implementation<core::control::command::storage::Create<StorageImplementations...>>::input
      ( IArchive& ia
      ) -> core::control::command::storage::Create<StorageImplementations...>
  {
    namespace command = core::control::command;
    using Create = command::storage::Create<StorageImplementations...>;

    auto implementation_id {load<decltype (Create::implementation_id)> (ia)};
    auto storage_parameter {load<decltype (Create::storage_parameter)> (ia)};

    return Create {implementation_id, storage_parameter};
  }
}
