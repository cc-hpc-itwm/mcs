// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/trace/Events.hpp>
#include <mcs/core/storage/tracer/record/Tag.hpp>
#include <mcs/util/not_null.hpp>
#include <vector>

namespace mcs::core::storage::tracer
{
  // A tracer that records all events.
  //
  template<is_implementation Storage>
    struct Record
  {
    using Tag = record::Tag;

    using Events = std::vector<typename trace::Events<Storage>::Variant>;

    struct Parameter
    {
      struct Create
      {
        util::not_null<Events> _events;
      };
    };

    explicit Record (Parameter::Create);

    template<typename Event, typename... Args>
      requires ( trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto trace (Args&&...) -> void;

  private:
    util::not_null<Events> _events;
  };
}

#include "detail/Record.ipp"
