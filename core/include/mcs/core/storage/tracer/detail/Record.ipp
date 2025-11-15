// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/execute_and_die_on_exception.hpp>
#include <utility>

namespace mcs::core::storage::tracer
{
  template<is_implementation Storage>
    Record<Storage>::Record (Parameter::Create create)
      : _events {create._events}
  {}
}

namespace mcs::core::storage::tracer
{
  template<is_implementation Storage>
    template<typename Event, typename... Args>
      requires ( trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto Record<Storage>::trace (Args&&... args) -> void
  {
    util::execute_and_die_on_exception
      ( "Could not record trace event"
      , [&]
        {
          _events->emplace_back
            ( std::in_place_type_t<Event>{}
            , std::forward<Args> (args)...
            );
        }
      );
  }
}
