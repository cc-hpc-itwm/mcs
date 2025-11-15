// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <fmt/base.h>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/storage/trace/event/Create.hpp>
#include <mcs/core/storage/trace/event/Destruct.hpp>
#include <mcs/core/storage/trace/event/chunk/Description.hpp>
#include <mcs/core/storage/trace/event/chunk/description/Result.hpp>
#include <mcs/core/storage/trace/event/file/Read.hpp>
#include <mcs/core/storage/trace/event/file/Write.hpp>
#include <mcs/core/storage/trace/event/file/read/Result.hpp>
#include <mcs/core/storage/trace/event/file/write/Result.hpp>
#include <mcs/core/storage/trace/event/segment/Create.hpp>
#include <mcs/core/storage/trace/event/segment/Remove.hpp>
#include <mcs/core/storage/trace/event/segment/create/Result.hpp>
#include <mcs/core/storage/trace/event/segment/remove/Result.hpp>
#include <mcs/core/storage/trace/event/size/Max.hpp>
#include <mcs/core/storage/trace/event/size/Used.hpp>
#include <mcs/core/storage/trace/event/size/max/Result.hpp>
#include <mcs/core/storage/trace/event/size/used/Result.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <utility>

namespace mcs::core::storage::trace::detail
{
  template<typename Tracer>
    concept has_constructor_with_parameter = requires
      ( typename Tracer::Parameter::Create parameter_create
      )
    {
      { Tracer { parameter_create
               }
      } -> std::convertible_to<Tracer>;
    };

  template<typename Tracer>
    concept has_serializable_and_formattable_tag =
       serialization::is_serializable<typename Tracer::Tag>
    && fmt::formattable<typename Tracer::Tag>
    ;

  template<typename Tracer, typename Storage, typename Event, typename... Args>
    concept handles = requires
      ( Tracer& tracer
      , Args&&... args
      )
    {
      { tracer.template trace<Event> (std::forward<Args> (args)...)
      } -> std::convertible_to<void>;
    };
}

namespace mcs::core::storage::trace
{
  // A class is a tracer for a storage if it handles all tracing
  // events in terms of their type and constructor arguments.
  //
  template<typename Tracer, typename Storage>
    concept is_tracer = is_implementation<Storage>
      && detail::has_constructor_with_parameter<Tracer>
      && detail::has_serializable_and_formattable_tag<Tracer>
      && detail::handles
         < Tracer
         , Storage
         , event::Create<Storage>
         , typename Storage::Parameter::Create
         >
      && detail::handles
         < Tracer
         , Storage
         , event::Destruct
         >
      && detail::handles
         < Tracer
         , Storage
         , event::chunk::Description<Storage, chunk::access::Const>
         , typename Storage::Parameter::Chunk::Description
         , core::storage::segment::ID
         , core::memory::Range
         >
      && detail::handles
         < Tracer
         , Storage
         , event::chunk::Description<Storage, chunk::access::Mutable>
         , typename Storage::Parameter::Chunk::Description
         , core::storage::segment::ID
         , core::memory::Range
         >
      && detail::handles
         < Tracer
         , Storage
         , event::chunk::description::Result<Storage, chunk::access::Const>
         , typename Storage::Chunk::template Description<chunk::access::Const>
         >
      && detail::handles
         < Tracer
         , Storage
         , event::chunk::description::Result<Storage, chunk::access::Mutable>
         , typename Storage::Chunk::template Description<chunk::access::Mutable>
         >
      && detail::handles
         < Tracer
         , Storage
         , event::file::Read<Storage>
         , typename Storage::Parameter::File::Read
         , core::storage::segment::ID
         , core::memory::Offset
         , std::filesystem::path
         , core::memory::Range
         >
      && detail::handles
         < Tracer
         , Storage
         , event::file::Write<Storage>
         , typename Storage::Parameter::File::Write
         , core::storage::segment::ID
         , core::memory::Offset
         , std::filesystem::path
         , core::memory::Range
         >
      && detail::handles
         < Tracer
         , Storage
         , event::file::read::Result
         , core::memory::Size
         >
      && detail::handles
         < Tracer
         , Storage
         , event::file::write::Result
         , core::memory::Size
         >
      && detail::handles
         < Tracer
         , Storage
         , event::segment::Create<Storage>
         , typename Storage::Parameter::Segment::Create
         , core::memory::Size
         >
      && detail::handles
         < Tracer
         , Storage
         , event::segment::Remove<Storage>
         , typename Storage::Parameter::Segment::Remove
         , core::storage::segment::ID
         >
      && detail::handles
         < Tracer
         , Storage
         , event::segment::create::Result
         , core::storage::segment::ID
         >
      && detail::handles
         < Tracer
         , Storage
         , event::segment::remove::Result
         , core::memory::Size
         >
      && detail::handles
         < Tracer
         , Storage
         , event::size::Max<Storage>
         , typename Storage::Parameter::Size::Max
         >
      && detail::handles
         < Tracer
         , Storage
         , event::size::Used<Storage>
         , typename Storage::Parameter::Size::Used
         >
      && detail::handles
         < Tracer
         , Storage
         , event::size::max::Result
         , core::storage::MaxSize
         >
      && detail::handles
         < Tracer
         , Storage
         , event::size::used::Result
         , core::memory::Size
         >
    ;
}
