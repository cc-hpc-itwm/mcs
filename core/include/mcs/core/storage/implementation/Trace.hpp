// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/implementation/trace/Tag.hpp>
#include <mcs/core/storage/implementation/trace/chunk/Description.hpp>
#include <mcs/core/storage/implementation/trace/parameter/Create.hpp>
#include <mcs/core/storage/trace/Concepts.hpp>
#include <mcs/core/storage/trace/Events.hpp>
#include <mcs/util/not_null.hpp>
#include <memory>

namespace mcs::core::storage::implementation
{
  // A Trace storage adds a tracer to some underlying storage. It
  // provides the same interface and behaves like the underlying
  // storage, except that it emits trace events to the tracer.
  //
  // The event handler of the Tracer will be called back two times for
  // each operation of the underlying storage:
  // - Before the operation is executed: To trace the parameters of the
  //   operation.
  // - After the operation has returned: To trace the result of the
  //   operation.
  //
  // The traced storage propagates exceptions produced when calling
  // the event handler of the tracer. In particular it will not
  // execute a traced operation if the tracer throws an exception for
  // the event produced before the execution.
  //
  // The trace storage owns the tracer and the storage.
  //
  // As one consequence there is no way to observe the tracer in
  // generic code as the generic code can not know the type of the
  // tracer. This implies that tracers can not have methods that are
  // not defined in their concept. In particular tracers can not have
  // observers for an owned state, instead every state must be
  // external, e.g. "file" or "reference to event log". This fits with
  // the design of storage where methods outside of their concept are
  // inaccessible in generic code.
  //
  // Another consequence of the design is that the storage can not be
  // used without also using the tracer. That can be considered an
  // advantage. However, to temporarily enable or disable tracing the
  // tracer itself must provide a method in its external state.
  //
  template<typename Tracer, is_implementation Storage>
    requires (storage::trace::is_tracer<Tracer, Storage>)
    struct Trace
  {
    using Tag = trace::Tag<Tracer, Storage>;

    struct Parameter
    {
      using Create = trace::parameter::Create<Tracer, Storage>;
      using Size = typename Storage::Parameter::Size;
      using Segment = typename Storage::Parameter::Segment;
      using Chunk = typename Storage::Parameter::Chunk;
      using File = typename Storage::Parameter::File;
    };
    using Error = typename Storage::Error;
    struct Chunk
    {
      template<core::chunk::is_access Access>
        using Description = trace::chunk::Description<Tracer, Storage, Access>;
    };

    explicit Trace (Parameter::Create);

    auto size_max
      ( Parameter::Size::Max
      ) const -> MaxSize
      ;
    auto size_used
      ( Parameter::Size::Used
      ) const -> memory::Size
      ;

    auto segment_create
      ( Parameter::Segment::Create
      , memory::Size
      ) -> segment::ID
      ;

    auto segment_remove
      ( Parameter::Segment::Remove
      , segment::ID
      ) -> memory::Size
      ;

    template<core::chunk::is_access Access>
      auto chunk_description
        ( Parameter::Chunk::Description
        , segment::ID
        , memory::Range
        ) const -> Chunk::template Description<Access>
      ;

    auto file_read
      ( Parameter::File::Read
      , storage::segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;
    auto file_write
      ( Parameter::File::Write
      , storage::segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;

  private:
    mutable Tracer _tracer;
    struct StorageDeleter
    {
      explicit StorageDeleter (util::not_null<Tracer>) noexcept;
      auto operator() (Storage*) const noexcept -> void;

    private:
      util::not_null<Tracer> _tracer;
    };
    std::unique_ptr<Storage, StorageDeleter> _storage;

    template<typename Event, typename... Args>
      requires (std::is_constructible_v<Event, Args...>)
      auto trace (Args&&...) const -> void;
  };
}

#include "detail/Trace.ipp"
