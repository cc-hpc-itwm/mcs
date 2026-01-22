// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <fmt/base.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>
#include <version>

namespace mcs::block_device
{
  // Storage to keep blocks.
  //
  struct Storage
  {
    util::ASIO::AnyConnectable _provider_connectable;
    core::storage::ID _storage_id;
    core::storage::Parameter _storage_parameter_chunk_description;
    core::storage::segment::ID _segment_id;
    core::memory::Range _range;

#ifdef _cpp_lib_constexpr_vector
    constexpr
#endif
              auto operator<=> (Storage const&) const = default;
  };

  // \note one could, in principle, detect whether or not storages
  // - are the same
  //   [--)
  //   [--)
  // - overlap
  //     [--)   [--)
  //    [--)     [--)
  // - touch
  //   [--)        [--)
  //      [--)  [--)
  // and use that information to join storages into larger ones.
}

namespace fmt
{
  template<>
    struct formatter<mcs::block_device::Storage>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::block_device::Storage const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::block_device::Storage>
  {
    using Type = mcs::block_device::Storage;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::block_device::Storage>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::block_device::Storage
        ;
  };
}

#include "detail/Storage.ipp"
