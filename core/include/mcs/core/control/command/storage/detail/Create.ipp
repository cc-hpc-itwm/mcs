// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <mcs/util/type/List.hpp>
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
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::control::command::storage::Create<StorageImplementations...>
      )
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation... StorageImplementations>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( create
      , context
      , mcs::core::control::command::storage::Create<StorageImplementations...>
      )
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
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
      ( state
      , core::control::command::storage::Create<StorageImplementations...>
      )
  {
    namespace command = core::control::command;
    using Create = command::storage::Create<StorageImplementations...>;

    return std::make_from_tuple<Create>
      ( parse<std::tuple< decltype (std::declval<Create>().implementation_id)
                        , decltype (std::declval<Create>().storage_parameter)
                        >
             >
          (state)
      );
  }
}

namespace mcs::serialization
{
  template<core::storage::is_implementation... StorageImplementations>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , create
      , core::control::command::storage::Create<StorageImplementations...>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, create, implementation_id);
    MCS_SERIALIZATION_SAVE_FIELD (oa, create, storage_parameter);

    return oa;
  }

  template<core::storage::is_implementation... StorageImplementations>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::control::command::storage::Create<StorageImplementations...>
      )
  {
    namespace command = core::control::command;
    using Create = command::storage::Create<StorageImplementations...>;

    MCS_SERIALIZATION_LOAD_FIELD (ia, implementation_id, Create);
    MCS_SERIALIZATION_LOAD_FIELD (ia, storage_parameter, Create);

    return Create {implementation_id, storage_parameter};
  }
}
