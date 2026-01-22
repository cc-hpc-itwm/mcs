// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdio>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <functional>
#include <iterator>
#include <mcs/config.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/read/STD/vector.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcs::util
{
  template<>
    struct Cast<::mcs_core_storage_memory_size, core::memory::Size>
  {
    auto operator()
      ( core::memory::Size
      ) const -> ::mcs_core_storage_memory_size
      ;
  };

  template<>
    struct Cast<::mcs_core_storage_memory_offset, core::memory::Offset>
  {
    auto operator()
      ( core::memory::Offset
      ) const -> ::mcs_core_storage_memory_offset
      ;
  };

  template<>
    struct Cast<::mcs_core_storage_memory_range, core::memory::Range>
  {
    auto operator()
      ( core::memory::Range
      ) const -> ::mcs_core_storage_memory_range
      ;
  };
}

namespace mcs::core::storage::implementation::detail
{
  auto vector_push_back
    ( void* sink
    , ::MCS_CORE_STORAGE_BYTE byte
    ) -> void
    ;

  auto vector_append
    ( void* sink
    , ::MCS_CORE_STORAGE_BYTE const* data
    , ::MCS_CORE_STORAGE_SIZE size
    ) -> void
    ;

  auto vector_reserve
    ( void* sink
    , ::MCS_CORE_STORAGE_SIZE size
    ) -> void
    ;

  auto channel
    ( util::not_null<std::vector<::MCS_CORE_STORAGE_BYTE>>
    ) -> ::mcs_core_storage_channel
    ;
}

namespace mcs::core::storage::implementation
{
  template<typename Fun, typename... Args>
    auto Import_C_API::invoke_and_throw_on_error (Fun&& fun, Args&&... args)
  {
    if (!fun)
    {
      throw Error::MethodNotProvided{};
    }

    auto error {std::vector<::MCS_CORE_STORAGE_BYTE>{}};

    auto const invocation
      { [&]
        {
          return std::invoke
            ( std::forward<Fun> (fun)
            , std::forward<Args> (args)...
            , detail::channel (std::addressof (error))
            );
        }
      };

    if constexpr (std::is_void_v<decltype (std::invoke (invocation))>)
    {
      std::invoke (invocation);

      if (!error.empty())
      {
        throw Error::Implementation {std::move (error)};
      }

      return;
    }
    else
    {
      auto r {std::invoke (invocation)};

      if (!error.empty())
      {
        throw Error::Implementation {std::move (error)};
      }

      return r;
    }
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto Import_C_API::chunk_description
      ( Parameter::Chunk::Description parameter_chunk_description
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  try
  {
    auto const implementation_memory_range
      { util::cast<::mcs_core_storage_memory_range> (memory_range)
      };
    auto const implementation_segment_id
      { util::cast<::mcs_core_storage_segment_id> (segment_id)
      };
    auto const implementation_parameter
      { ::mcs_core_storage_parameter
        { util::cast<::MCS_CORE_STORAGE_BYTE const*>
           ( parameter_chunk_description._parameter_chunk_description.data()
           )
        , parameter_chunk_description._parameter_chunk_description.size()
        }
      };
    auto description {std::vector<::MCS_CORE_STORAGE_BYTE>{}};

    invoke_and_throw_on_error
      ( c_api::Traits<Access>::chunk_description (_storage)
      , *_instance
      , implementation_parameter
      , implementation_segment_id
      , implementation_memory_range
      , detail::channel (std::addressof (description))
      );

    return Chunk::Description<Access>
      { std::move (description)
      , std::addressof (_storage)
      , _instance.get()
      };
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::Chunk::Description<Access>
          { parameter_chunk_description
          , segment_id
          , memory_range
          }
      );
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto Import_C_API::Chunk::Description<Access>::State::Deleter::operator()
      ( typename Import_C_API::Chunk::Description<Access>::State::ChunkState*
          chunk_state
      ) const -> void
  {
    auto const default_delete
      { nonstd::make_scope_exit_that_dies_on_exception
        ( "Import_C_API::Description::State::Deleter"
        , [&]
          {
            std::default_delete
              < typename Import_C_API::Chunk::Description<Access>::State::ChunkState
              >{} (chunk_state)
              ;
          }
        )
      };

    Import_C_API::invoke_and_throw_on_error
      ( c_api::Traits<Access>::chunk_state_destruct (*_storage)
      , *_instance
      , *chunk_state
      );
  }

  template<chunk::is_access Access>
    Import_C_API::Chunk::Description<Access>::State::State
      ( Description<Access> const& description
      )
        : _chunk_state
          { std::invoke
            ( [&]
              {
                auto const chunk_description
                  { ::mcs_core_storage_parameter
                    { util::cast<::MCS_CORE_STORAGE_BYTE const*>
                        ( description._description.data()
                        )
                    , description._description.size()
                    }
                  };

                return new ChunkState
                  { Import_C_API::invoke_and_throw_on_error
                    ( c_api::Traits<Access>::chunk_state (*description._storage)
                    , *description._instance
                    , chunk_description
                    )
                  };
              }
            )
          , Deleter {description._storage, description._instance}
          }
  {}

  template<chunk::is_access Access>
    auto Import_C_API::Chunk::Description<Access>::State::bytes
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _bytes;
  }

  template<chunk::is_access Access>
    Import_C_API::Chunk::Description<Access>::Description
      ( std::vector<::MCS_CORE_STORAGE_BYTE> description
      , util::not_null<::mcs_core_storage const> storage
      , util::not_null<::mcs_core_storage_instance const> instance
      ) noexcept
        : _description {std::move (description)}
        , _storage {storage}
        , _instance {instance}
  {}
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Implementation::error
    (
    ) const noexcept -> Bytes const&
  {
    return _error;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Create::parameter
    (
    ) const noexcept -> Parameter::Create const&
  {
    return _parameter;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Size::Max::parameter
    (
    ) const noexcept -> Parameter::Size::Max const&
  {
    return _parameter;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Size::Used::parameter
    (
    ) const noexcept -> Parameter::Size::Used const&
  {
    return _parameter;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Segment::Create::parameter
    (
    ) const noexcept -> Parameter::Segment::Create const&
  {
    return _parameter;
  }
  constexpr auto Import_C_API::Error::Segment::Create::size
    (
    ) const noexcept -> memory::Size
  {
    return _size;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::Segment::Remove::parameter
    (
    ) const noexcept -> Parameter::Segment::Remove const&
  {
    return _parameter;
  }
  constexpr auto Import_C_API::Error::Segment::Remove::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::File::Read::parameter
    (
    ) const noexcept -> Parameter::File::Read const&
  {
    return _parameter;
  }
  constexpr auto Import_C_API::Error::File::Read::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }
  constexpr auto Import_C_API::Error::File::Read::offset
    (
    ) const noexcept -> memory::Offset
  {
    return _offset;
  }
  constexpr auto Import_C_API::Error::File::Read::path
    (
    ) const noexcept -> std::filesystem::path const&
  {
    return _path;
  }
  constexpr auto Import_C_API::Error::File::Read::range
    (
    ) const noexcept -> memory::Range
  {
    return _range;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::File::Write::parameter
    (
    ) const noexcept -> Parameter::File::Write const&
  {
    return _parameter;
  }
  constexpr auto Import_C_API::Error::File::Write::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }
  constexpr auto Import_C_API::Error::File::Write::offset
    (
    ) const noexcept -> memory::Offset
  {
    return _offset;
  }
  constexpr auto Import_C_API::Error::File::Write::path
    (
    ) const noexcept -> std::filesystem::path const&
  {
    return _path;
  }
  constexpr auto Import_C_API::Error::File::Write::range
    (
    ) const noexcept -> memory::Range
  {
    return _range;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    Import_C_API::Error::Chunk::Description<Access>::Description
      ( Parameter::Chunk::Description parameter
      , segment::ID segment_id
      , memory::Range range
      )
        : mcs::Error
          { fmt::format
              ( "storage::Import_C_API::Chunk::Description<{}> ({}, {}, {})"
              , Access{}
              , parameter
              , segment_id
              , range
              )
          }
        , _parameter {parameter}
        , _segment_id {segment_id}
        , _range {range}
  {}
#if defined (MCS_CONFIG_CLANG_TEMPLATED_DTOR_PARSER_IS_BROKEN)
  template<chunk::is_access Access>
    Import_C_API::Error::Chunk::Description<Access>::~Description<Access>() = default;
#else
  template<chunk::is_access Access>
    Import_C_API::Error::Chunk::Description<Access>::~Description() = default;
#endif
}

namespace mcs::core::storage::implementation
{
  constexpr auto Import_C_API::Error::BadAlloc::requested
    (
    ) const noexcept -> memory::Size
  {
    return _requested;
  }

  constexpr auto Import_C_API::Error::BadAlloc::used
    (
    ) const noexcept -> memory::Size
  {
    return _used;
  }

  constexpr auto Import_C_API::Error::BadAlloc::max
    (
    ) const noexcept -> MaxSize
  {
    return _max;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Tag>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Tag>::format
      ( mcs::core::storage::implementation::Import_C_API::Tag const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::storage::implementation::Import_C_API"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Tag>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Tag
  {
    prefix (state, "mcs::core::storage::implementation::Import_C_API");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Tag>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::Size::Max"
      , std::make_tuple (value._parameter_size_max)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max
  {
    prefix (state, "Import_C_API::Size::Max");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max::_parameter_size_max)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::Size::Used"
      , std::make_tuple (value._parameter_size_used)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used
  {
    prefix (state, "Import_C_API::Size::Used");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used::_parameter_size_used)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::Segment::Create "
      , std::make_tuple (value._parameter_segment_create)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create
  {
    prefix (state, "Import_C_API::Segment::Create ");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create::_parameter_segment_create)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::Segment::Remove "
      , std::make_tuple (value._parameter_segment_remove)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove
  {
    prefix (state, "Import_C_API::Segment::Remove ");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove::_parameter_segment_remove)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::Chunk::Description"
      , std::make_tuple (value._parameter_chunk_description)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description
  {
    prefix (state, "Import_C_API::Chunk::Description");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description::_parameter_chunk_description)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::File::Read>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::File::Read>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Read const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::File::Read"
      , std::make_tuple (value._parameter_file_read)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::File::Read>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::File::Read
  {
    prefix (state, "Import_C_API::File::Read");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::File::Read>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::File::Read::_parameter_file_read)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::File::Write>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Parameter::File::Write>::format
      ( mcs::core::storage::implementation::Import_C_API::Parameter::File::Write const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Import_C_API::File::Write"
      , std::make_tuple (value._parameter_file_write)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Import_C_API::Parameter::File::Write>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Import_C_API::Parameter::File::Write
  {
    prefix (state, "Import_C_API::File::Write");

    return std::make_from_tuple<mcs::core::storage::implementation::Import_C_API::Parameter::File::Write>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Import_C_API::Parameter::File::Write::_parameter_file_write)>> (state));
  }
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Import_C_API::Chunk::Description<Access>>::output
      ( OArchive& oa
      , core::storage::implementation::Import_C_API::Chunk::Description<Access> const& description
      ) -> OArchive&
  {
    save (oa, description._description);
    save (oa, util::cast<std::uintmax_t> (description._storage.get()));
    save (oa, util::cast<std::uintmax_t> (description._instance.get()));

    return oa;
  }

  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Import_C_API::Chunk::Description<Access>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::Import_C_API::Chunk::Description<Access>
  {
    using Description
      = core::storage::implementation::Import_C_API::Chunk::Description<Access>
      ;

    auto _description {load<decltype (Description::_description)> (ia)};
    auto const storage {load<std::uintmax_t> (ia)};
    auto const instance {load<std::uintmax_t> (ia)};

    return Description
      { _description
      , util::cast<::mcs_core_storage const*> (storage)
      , util::cast<::mcs_core_storage_instance const*> (instance)
      };
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access>>::format
        ( mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access> const& description
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "Import_C_API::Chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( description._description
        )
      );
  }
}
