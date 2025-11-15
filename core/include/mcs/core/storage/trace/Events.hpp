// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/core/storage/Concepts.hpp>
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
#include <mcs/util/type/List.hpp>

namespace mcs::core::storage::trace
{
  template<is_implementation Storage>
    using Events = util::type::List
      < trace::event::Create<Storage>
      , trace::event::Destruct
      , trace::event::chunk::Description<Storage, chunk::access::Const>
      , trace::event::chunk::Description<Storage, chunk::access::Mutable>
      , trace::event::chunk::description::Result<Storage, chunk::access::Const>
      , trace::event::chunk::description::Result<Storage, chunk::access::Mutable>
      , trace::event::file::Read<Storage>
      , trace::event::file::Write<Storage>
      , trace::event::file::read::Result
      , trace::event::file::write::Result
      , trace::event::segment::Create<Storage>
      , trace::event::segment::Remove<Storage>
      , trace::event::segment::create::Result
      , trace::event::segment::remove::Result
      , trace::event::size::Max<Storage>
      , trace::event::size::Used<Storage>
      , trace::event::size::max::Result
      , trace::event::size::used::Result
      >;

  template<typename Storage, typename Event>
    concept is_event = Events<Storage>::template contains<Event>();
}
