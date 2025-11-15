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
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/read/STD/vector.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/tuplish/define.hpp>
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

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "mcs::core::storage::implementation::Import_C_API"
  , mcs::core::storage::implementation::Import_C_API::Tag
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::Size::Max"
  , mcs::core::storage::implementation::Import_C_API::Parameter::Size::Max
  , _parameter_size_max
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::Size::Used"
  , mcs::core::storage::implementation::Import_C_API::Parameter::Size::Used
  , _parameter_size_used
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::Segment::Create "
  , mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Create
  , _parameter_segment_create
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::Segment::Remove "
  , mcs::core::storage::implementation::Import_C_API::Parameter::Segment::Remove
  , _parameter_segment_remove
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::Chunk::Description"
  , mcs::core::storage::implementation::Import_C_API::Parameter::Chunk::Description
  , _parameter_chunk_description
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::File::Read"
  , mcs::core::storage::implementation::Import_C_API::Parameter::File::Read
  , _parameter_file_read
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Import_C_API::File::Write"
  , mcs::core::storage::implementation::Import_C_API::Parameter::File::Write
  , _parameter_file_write
  );

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::storage::implementation::Import_C_API::Chunk::Description<Access>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, _description);
    save (oa, util::cast<std::uintmax_t> (description._storage.get()));
    save (oa, util::cast<std::uintmax_t> (description._instance.get()));

    return oa;
  }

  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::Import_C_API::Chunk::Description<Access>
      )
  {
    using Description
      = core::storage::implementation::Import_C_API::Chunk::Description<Access>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _description, Description);
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
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access>
      )
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( description
      , context
      , mcs::core::storage::implementation::Import_C_API::Chunk::Description<Access>
      )
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
