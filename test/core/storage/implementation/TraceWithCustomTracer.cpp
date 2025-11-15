// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <concepts>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <list>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/Trace.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Heap/Chunk/Description.hpp>
#include <mcs/testing/core/operator==/storage/implementation/Heap/Parameter.hpp>
#include <mcs/testing/core/operator==/storage/trace/Events.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <type_traits>
#include <variant>

namespace
{
  // For the test the event sink is not just a collection of events
  // but provides assertions, too
  template<mcs::core::storage::is_implementation Storage>
    struct EventSink
  {
    using Events = std::list
      < typename mcs::core::storage::trace::Events<Storage>::Variant
      >;

    template<typename Event, typename... Args>
      requires ( mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto record (Args&&...) -> void;


    template<typename Event, typename... Args>
      requires ( mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto assert_back_is (Args&&...) -> void;

    auto assert_is_empty() -> void;

  private:
    Events _events;
  };
}

namespace
{
  struct CustomTracerTag{};

  template<mcs::core::storage::is_implementation Storage>
    struct CustomTracer
  {
    using Tag = CustomTracerTag;

    struct Parameter
    {
      struct Create
      {
        mcs::util::not_null<EventSink<Storage>> _event_sink;
      };
    };

    explicit CustomTracer (Parameter::Create);

    template<typename Event, typename... Args>
      requires ( mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto trace (Args&&...) -> void;

    struct Error : mcs::Error
    {
      Error();
      MCS_ERROR_COPY_MOVE_DEFAULT (Error);
    };

  private:
    mcs::util::not_null<EventSink<Storage>> _event_sink;
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (CustomTracerTag);
}

namespace mcs::core
{
  namespace
  {
    struct MCSStorageTracingR : public testing::random::Test{};
  }

  TEST_F ( MCSStorageTracingR
         , custom_tracer_works_and_tracer_exceptions_are_propagated
         )
  {
    using TestingStorage = testing::core::storage::implementation::Heap;
    using Storage = typename TestingStorage::Storage;

    using RandomSizeT = testing::random::value<std::size_t>;

    auto const size {std::invoke (RandomSizeT {2 << 10, 2 << 20})};
    auto const segment_size {std::invoke (RandomSizeT {2 << 10, size})};
    auto const range
      { std::invoke
        ( [&]
          {
            auto const o {std::invoke (RandomSizeT {0, segment_size - 1})};
            auto const s {std::invoke (RandomSizeT {1, segment_size - o})};
            return memory::make_range
              ( memory::make_offset (o)
              , memory::make_size (s)
              );
          }
        )
      };

    auto const max_size {storage::MaxSize::Limit {memory::make_size (size)}};
    auto const testing_storage
      { TestingStorage {"TraceWithCustomTracer", max_size}
      };

    using TracedStorage
      = storage::implementation::Trace<CustomTracer<Storage>, Storage>
      ;

    auto event_sink {EventSink<Storage>{}};

#define MCS_TEST_CORE_ASSERT_BACK_IS(_type...)                  \
    SCOPED_TRACE (#_type); event_sink.assert_back_is<_type>

    {
      // create the storage
      auto traced_storage
        { TracedStorage
          { typename TracedStorage::Parameter::Create
            { typename CustomTracer<Storage>::Parameter::Create
              { std::addressof (event_sink)
              }
            , testing_storage.parameter_create()
            }
          }
        };

      MCS_TEST_CORE_ASSERT_BACK_IS (storage::trace::event::Create<Storage>)
        ( testing_storage.parameter_create()
        );

      // query size_max
      ASSERT_EQ
        ( max_size
        , traced_storage.size_max
          ( testing_storage.parameter_size_max()
          )
        );

      MCS_TEST_CORE_ASSERT_BACK_IS (storage::trace::event::size::max::Result)
        ( max_size
        );
      MCS_TEST_CORE_ASSERT_BACK_IS (storage::trace::event::size::Max<Storage>)
        ( testing_storage.parameter_size_max()
        );

      // the event size_used causes the tracer to throw -> the event
      // size_used is tested in Trace.cpp
      testing::require_exception
        ( [&]
          {
            std::ignore = traced_storage.size_used
              ( testing_storage.parameter_size_used()
              );
          }
        , testing::assert_type<typename CustomTracer<Storage>::Error>()
        );

      // create a segment
      auto const segment_id
        { traced_storage.segment_create
          ( testing_storage.parameter_segment_create()
          , memory::make_size (segment_size)
          )
        };

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::segment::create::Result
        )
        ( segment_id
        );
      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::segment::Create<Storage>
        )
        ( testing_storage.parameter_segment_create()
        , memory::make_size (segment_size)
        );

      // retrieve a const chunk_description
      auto const const_chunk_description
        { traced_storage.template chunk_description<chunk::access::Const>
            ( testing_storage.parameter_chunk_description()
            , segment_id
            , range
            )
        };

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::chunk::description::Result
            < Storage
            , chunk::access::Const
            >
        )
        ( const_chunk_description
        );

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::chunk::Description
          < Storage
          , chunk::access::Const
          >
        )
        ( testing_storage.parameter_chunk_description()
        , segment_id
        , range
        );

      // retrieve a mutable chunk_description
      auto const mutable_chunk_description
        { traced_storage.template chunk_description<chunk::access::Mutable>
            ( testing_storage.parameter_chunk_description()
            , segment_id
            , range
            )
        };

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::chunk::description::Result
          < Storage
          , chunk::access::Mutable
          >
        )
        ( mutable_chunk_description
        );

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::chunk::Description
          < Storage
          , chunk::access::Mutable
          >
        )
        ( testing_storage.parameter_chunk_description()
        , segment_id
        , range
        );

      auto const temporary_directory
        { testing::UniqTemporaryDirectory {"TraceWithCustomTracer"}
        };

      // file_write
      ASSERT_EQ
        ( memory::size (range)
        , traced_storage.file_write
          ( testing_storage.parameter_file_write()
          , segment_id
          , memory::make_offset (0)
          , temporary_directory.path() / "data"
          , range
          )
        );

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::file::write::Result
        )
        ( memory::size (range)
        );
      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::file::Write<Storage>
        )
        ( testing_storage.parameter_file_write()
        , segment_id
        , memory::make_offset (0)
        , temporary_directory.path() / "data"
        , range
        );

      // file_read
      ASSERT_EQ
        ( memory::size (range)
        , traced_storage.file_read
          ( testing_storage.parameter_file_read()
          , segment_id
          , memory::make_offset (0)
          , temporary_directory.path() / "data"
          , range
          )
        );

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::file::read::Result
        )
        ( memory::size (range)
        );
      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::file::Read<Storage>
        )
        ( testing_storage.parameter_file_read()
        , segment_id
        , memory::make_offset (0)
        , temporary_directory.path() / "data"
        , range
        );

      // remove the segment
      ASSERT_EQ
        ( memory::make_size (segment_size)
        , traced_storage.segment_remove
          ( testing_storage.parameter_segment_remove()
          , segment_id
          )
        );

      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::segment::remove::Result
        )
        ( memory::make_size (segment_size)
        );
      MCS_TEST_CORE_ASSERT_BACK_IS
        ( storage::trace::event::segment::Remove<Storage>
        )
        ( testing_storage.parameter_segment_remove()
        , segment_id
        );
    }

    MCS_TEST_CORE_ASSERT_BACK_IS (storage::trace::event::Destruct)();

    // all events have been verified
    event_sink.assert_is_empty();

#undef MCS_TEST_CORE_ASSERT_BACK_IS
  }
}

namespace
{
  template<mcs::core::storage::is_implementation Storage>
    template<typename Event, typename... Args>
      requires (  mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
    auto EventSink<Storage>::record (Args&&... args) -> void
  {
    _events.emplace_back
      ( std::in_place_type_t<Event>{}
      , std::forward<Args> (args)...
      );
  }

  template<mcs::core::storage::is_implementation Storage>
    template<typename Event, typename... Args>
      requires (  mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
    auto EventSink<Storage>::assert_back_is (Args&&... args) -> void
  {
    ASSERT_TRUE (!_events.empty());
    ASSERT_TRUE (std::holds_alternative<Event> (_events.back()));
    ASSERT_EQ ( std::get<Event> (_events.back())
              , Event {std::forward<Args> (args)...}
              );
    _events.pop_back();
  }

  template<mcs::core::storage::is_implementation Storage>
    auto EventSink<Storage>::assert_is_empty() -> void
  {
    ASSERT_TRUE (_events.empty());
  }
}

namespace
{
  template<mcs::core::storage::is_implementation Storage>
    CustomTracer<Storage>::CustomTracer (Parameter::Create create)
      : _event_sink {create._event_sink}
  {}

  template<mcs::core::storage::is_implementation Storage>
    template<typename Event, typename... Args>
      requires ( mcs::core::storage::trace::is_event<Storage, Event>
               && std::constructible_from<Event, Args...>
               )
      auto CustomTracer<Storage>::trace (Args&&... args) -> void
  {
    if constexpr
      ( std::is_same_v< Event
                      , mcs::core::storage::trace::event::size::Used<Storage>
                      >
      )
    {
      throw Error{};
    }

    _event_sink->template record<Event> (std::forward<Args> (args)...);
  }
}

namespace
{
  template<mcs::core::storage::is_implementation Storage>
    CustomTracer<Storage>::Error::Error()
      : mcs::Error {"CustomTracer::Error"}
  {}
  template<mcs::core::storage::is_implementation Storage>
    CustomTracer<Storage>::Error::~Error() = default;
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, CustomTracerTag)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT
    ( /* custom_tracer_tag */
    , context
    , CustomTracerTag
    )
  {
    return fmt::format_to (context.out(), "CustomTracer");
  }
}
