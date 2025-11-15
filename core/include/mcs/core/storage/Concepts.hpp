// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::core::storage
{
  namespace detail
  {
    template<typename SI>
      concept has_constructor_with_parameter = requires
        ( typename SI::Parameter::Create parameter_create
        )
      {
        { SI { parameter_create
             }
        } -> std::convertible_to<SI>;
      };

    template<typename SI>
      concept has_size_max = requires
        ( SI const& si
        , typename SI::Parameter::Size::Max parameter_size_max
        )
      {
        { si.size_max ( parameter_size_max
                      )
        } -> std::convertible_to<MaxSize>;
      };

    template<typename SI>
      concept has_size_used = requires
        ( SI const& si
        , typename SI::Parameter::Size::Used parameter_size_used
        )
      {
        { si.size_used ( parameter_size_used
                       )
        } -> std::convertible_to<memory::Size>;
      };

    template<typename SI>
      concept has_segment_create = requires
        ( SI& si
        , typename SI::Parameter::Segment::Create parameter_segment_create
        , memory::Size memory_size
        )
      {
        { si.segment_create ( parameter_segment_create
                            , memory_size
                            )
        } -> std::convertible_to<segment::ID>;
      };

    template<typename SI>
      concept has_segment_remove = requires
        ( SI& si
        , typename SI::Parameter::Segment::Remove parameter_segment_remove
        , segment::ID segment_id
        )
      {
        { si.segment_remove ( parameter_segment_remove
                            , segment_id
                            )
        } -> std::convertible_to<memory::Size>;
      };

    template<typename SI, typename Access>
      concept has_chunk_description = requires
        ( SI const& si
        , typename SI::Parameter::Chunk::Description parameter_chunk_description
        , segment::ID segment_id
        , memory::Range memory_range
        )
      {
        { si.template chunk_description<Access>
            ( parameter_chunk_description
            , segment_id
            , memory_range
            )
        } -> std::convertible_to
               < typename SI::Chunk::template Description<Access>
               >;
       }
      ;

    template<typename SI, typename Access>
      concept has_chunk_state = requires
        ( typename SI::Chunk::template Description<Access> const& chunk_description
        , typename SI::Chunk::template Description<Access>::State const& chunk_state
        )
      {
        { typename SI::Chunk::template Description<Access>::State {chunk_description}
        };

        { chunk_state.bytes()
        } -> std::convertible_to<typename Access::template Span<std::byte>>;
      }
      ;

    template<typename SI>
      concept has_file_read = requires
        ( SI const& si
        , typename SI::Parameter::File::Read parameter_file_read
        , segment::ID segment_id
        , memory::Offset offset
        , std::filesystem::path path
        , memory::Range range
        )
      {
        { si.file_read ( parameter_file_read
                       , segment_id
                       , offset
                       , path
                       , range
                       )
        } -> std::convertible_to<memory::Size>;
      };

    template<typename SI>
      concept has_file_write = requires
        ( SI const& si
        , typename SI::Parameter::File::Write parameter_file_write
        , segment::ID segment_id
        , memory::Offset offset
        , std::filesystem::path path
        , memory::Range range
        )
      {
        { si.file_write ( parameter_file_write
                        , segment_id
                        , offset
                        , path
                        , range
                        )
        } -> std::convertible_to<memory::Size>;
      };

    template<typename SI>
      concept has_error_bad_alloc = requires
        ( typename SI::Error::BadAlloc const& bad_alloc
        )
      {
        {bad_alloc.requested()} -> std::convertible_to<memory::Size>;
        {bad_alloc.used()} -> std::convertible_to<memory::Size>;
        {bad_alloc.max()} -> std::convertible_to<MaxSize>;
      };
  }

  template<typename SI>
    concept is_implementation =
       detail::has_constructor_with_parameter<SI>
    && detail::has_size_max<SI>
    && detail::has_size_used<SI>
    && detail::has_segment_create<SI>
    && detail::has_segment_remove<SI>
    && detail::has_chunk_description<SI, chunk::access::Const>
    && detail::has_chunk_description<SI, chunk::access::Mutable>
    && detail::has_chunk_state<SI, chunk::access::Const>
    && detail::has_chunk_state<SI, chunk::access::Mutable>
    && detail::has_file_read<SI>
    && detail::has_file_write<SI>
    && serialization::is_serializable<typename SI::Tag>
    && serialization::is_serializable<typename SI::Parameter::Size::Max>
    && serialization::is_serializable<typename SI::Parameter::Size::Used>
    && serialization::is_serializable<typename SI::Parameter::Segment::Create>
    && serialization::is_serializable<typename SI::Parameter::Segment::Remove>
    && serialization::is_serializable<typename SI::Parameter::Chunk::Description>
    && serialization::is_serializable<typename SI::Parameter::File::Read>
    && serialization::is_serializable<typename SI::Parameter::File::Write>
    && fmt::formattable<typename SI::Tag>
    && fmt::formattable<typename SI::Parameter::Size::Max>
    && fmt::formattable<typename SI::Parameter::Size::Used>
    && fmt::formattable<typename SI::Parameter::Segment::Create>
    && fmt::formattable<typename SI::Parameter::Segment::Remove>
    && fmt::formattable<typename SI::Parameter::Chunk::Description>
    && fmt::formattable<typename SI::Parameter::File::Read>
    && fmt::formattable<typename SI::Parameter::File::Write>
    && detail::has_error_bad_alloc<SI>
    && serialization::is_serializable
         < typename SI::Chunk::template Description<chunk::access::Const>
         >
    && serialization::is_serializable
         < typename SI::Chunk::template Description<chunk::access::Mutable>
         >
    && fmt::is_formattable
         < typename SI::Chunk::template Description<chunk::access::Const>
         >::value
    && fmt::is_formattable
         < typename SI::Chunk::template Description<chunk::access::Mutable>
         >::value
    ;
}
