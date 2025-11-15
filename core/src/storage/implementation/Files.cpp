// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <linux/magic.h>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/Copy.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/statfs.hpp>
#include <mcs/util/touch.hpp>
#include <utility>

namespace
{
  auto is_nfs_mount (std::filesystem::path const& path) -> bool
  {
    return mcs::util::syscall::statfs (path.c_str()).f_type == NFS_SUPER_MAGIC;
  }
}

namespace mcs::core::storage::implementation
{
  Files::Error::BadAlloc::BadAlloc
    ( memory::Size requested
    , memory::Size used
    , MaxSize max
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Files::BadAlloc: requested {}, used {}, max {}"
          , requested
          , used
          , max
          )
        }
      , _requested {requested}
      , _used {used}
      , _max {max}
  {}
  Files::Error::BadAlloc::~BadAlloc() = default;

  Files::Error::Create::Create (Parameter::Create parameter_create)
    : mcs::Error
      { fmt::format ( "storage::implementation::Files::Files: {}"
                    , parameter_create
                    )
      }
    , _parameter_create {parameter_create}
  {}
  auto Files::Error::Create::parameter() const -> Parameter::Create
  {
    return _parameter_create;
  }
  Files::Error::Create::~Create() = default;

  Files::Error::Create::PrefixDoesNotExist::PrefixDoesNotExist (Prefix prefix)
    : mcs::Error
      { fmt::format
         ( "storage::implementation::Files::PrefixDoesNotExist: '{}'"
         , prefix
         )
      }
    , _prefix {prefix}
  {}
  auto Files::Error::Create::PrefixDoesNotExist::prefix() const -> Prefix
  {
    return _prefix;
  }
  Files::Error::Create::PrefixDoesNotExist::~PrefixDoesNotExist() = default;

  Files::Error::Create::UnsupportedNFSMount::UnsupportedNFSMount (Prefix prefix)
    : mcs::Error
      { fmt::format
         ( "storage::implementation::Files::UnsupportedNFSMount: '{}'"
         , prefix
         )
      }
    , _prefix {prefix}
  {}
  auto Files::Error::Create::UnsupportedNFSMount::prefix() const -> Prefix
  {
    return _prefix;
  }
  Files::Error::Create::UnsupportedNFSMount::~UnsupportedNFSMount() = default;

  Files::Error::Create::PrefixContainsNonSegmentFile::PrefixContainsNonSegmentFile
    ( Prefix prefix
    , std::filesystem::path non_segment_file
    )
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Files::PrefixContainsNonSegmentFile: prefix '{}', file '{}'"
          , prefix
          , non_segment_file
          )
      }
    , _prefix {prefix}
    , _non_segment_file {non_segment_file}
  {}
  auto Files::Error::Create::PrefixContainsNonSegmentFile::prefix() const -> Prefix
  {
    return _prefix;
  }
  auto Files::Error::Create::PrefixContainsNonSegmentFile::non_segment_file
    (
    ) const -> std::filesystem::path
  {
    return _non_segment_file;
  }
  Files::Error::Create::PrefixContainsNonSegmentFile::~PrefixContainsNonSegmentFile
    (
    ) = default;
}

namespace mcs::core::storage::implementation
{
  Files::Error::ChunkDescription::ChunkDescription
    ( Parameter::Chunk::Description parameter
    , segment::ID segment_id
    , memory::Range memory_range
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Files::ChunkDescription"
            ": segment_id '{}'"
            ", memory_range '{}'"
            ", parameter '{}'"
          , segment_id
          , memory_range
          , parameter
          )
        }
      , _parameter {parameter}
      , _segment_id {segment_id}
      , _memory_range {memory_range}
  {}
  Files::Error::ChunkDescription::~ChunkDescription() = default;
}

namespace mcs::core::storage::implementation
{
  Files::Error::ChunkDescription::UnknownSegmentID::UnknownSegmentID() noexcept
    : mcs::Error {"Unknown segment id"}
  {}
  Files::Error::ChunkDescription::UnknownSegmentID::~UnknownSegmentID
    (
    ) = default
    ;
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Files::Prefix
  , value
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::storage::implementation::Files::Parameter::Create
  , prefix
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Files::Parameter::Segment::Create
  , persistency
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Files::Parameter::Segment::Remove
  , force_removal
  );

namespace mcs::core::storage::implementation
{
  Files::Files (Parameter::Create create)
  try
    : _prefix {create.prefix}
    , _max_size {create.max_size}
  {
    if (!std::filesystem::exists (_prefix.value))
    {
      throw Error::Create::PrefixDoesNotExist {_prefix};
    }

    if (is_nfs_mount (_prefix.value))
    {
      throw Error::Create::UnsupportedNFSMount {_prefix};
    }

    for ( auto file {std::filesystem::directory_iterator (_prefix.value)}
        ; file != std::filesystem::directory_iterator{}
        ; ++file
        )
    {
      auto segment_id
        { [&]
          {
            try
            {
              return util::read::read<segment::ID>
                (file->path().filename().string());
            }
            catch (...)
            {
              std::throw_with_nested
                ( Error::Create::PrefixContainsNonSegmentFile
                    { _prefix
                    , file->path()
                    }
                );
            }
          }()
        };

      // \note Recovered segments have been kept in the past and will,
      // again, be kept by default. They can be removed using the
      // ForceRemoval option in Remove.
      //
      segment_use
        ( Parameter::Segment::Create {Parameter::Segment::OnRemove::Keep{}}
        , segment_id
        , memory::make_size (std::filesystem::file_size (file->path()))
        );

      _next_segment_id = std::max (_next_segment_id, ++segment_id);
    }
  }
  catch (...)
  {
    std::throw_with_nested (Error::Create {create});
  }

  auto Files::size_max
    ( Parameter::Size::Max
    ) const -> MaxSize
  {
    return _max_size;
  }
  auto Files::size_used
    ( Parameter::Size::Used
    ) const -> memory::Size
  {
    return _size_used;
  }

  auto Files::segment_create
    ( Parameter::Segment::Create parameter
    , memory::Size size
    ) ->  segment::ID
  {
    auto const increment_segment_id
      {nonstd::make_scope_success ([&]() noexcept { ++_next_segment_id; })};

    segment_use (parameter, _next_segment_id, size);

    return _next_segment_id;
  }

  auto Files::segment_use
    ( Parameter::Segment::Create parameter
    , segment::ID segment_id
    , memory::Size size
    ) -> void
  {
    auto const increment_size_used
      {nonstd::make_scope_success ([&]() noexcept { _size_used += size; })};

    if (_size_used + size > _max_size)
    {
      throw Error::BadAlloc {size, _size_used, _max_size};
    }

    if (! _file_by_id.try_emplace
            ( segment_id
            , new OpenFileImpl<chunk::access::Mutable>
              { filename (segment_id)
              , parameter.persistency
              , size
              }
            , typename OpenFileImpl<chunk::access::Mutable>::Deleter{}
            ).second
       )
    {
      throw mcs::Error
        { fmt::format ("Duplicate segment id '{}'", segment_id)
        };
    }
  }

  auto Files::filename (segment::ID segment_id) const -> std::filesystem::path
  {
    return _prefix.value / fmt::format ("{}", segment_id);
  }

  auto Files::segment_remove
    ( Parameter::Segment::Remove segment_remove
    , segment::ID segment_id
    ) -> memory::Size
  {
    auto& file {_file_by_id.at (segment_id)};

    if (segment_remove.force_removal.has_value())
    {
      file->remove_on_destruction();
    }

    auto const size_freed
      { std::visit
        ( util::overloaded
          ( [] (Parameter::Segment::OnRemove::Keep)
            {
              // do not count down, the space is not freed
              return memory::make_size (0);
            }
          , [&] (Parameter::Segment::OnRemove::Remove)
            {
              return memory::make_size (file->data().size());
            }
          )
        , file->_persistency
        )
      };

    _size_used -= size_freed;

    _file_by_id.erase (segment_id);

    return size_freed;
  }

  auto Files::file_read
    ( Parameter::File::Read
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    auto const max_offset
      { memory::make_offset (_file_by_id.at (segment_id)->_size)
      };

    if (max_offset < offset + size (range))
    {
      // \todo specific exception
      throw std::out_of_range
        { fmt::format ( "File::file_read: {} is not completely inside of {}"
                      , make_range (offset, size (range))
                      , make_range (memory::make_offset (0), max_offset)
                      )
        };
    }

    return memory::make_size
      ( util::Copy{}
        ( util::copy::FileReadLocation
            {path, make_off_t (begin (range))}
        , util::copy::FileWriteLocation
            {util::touch (filename (segment_id)), make_off_t (offset)}
        , size_cast<std::size_t> (size (range))
        )
      );
  }

  auto Files::file_write
    ( Parameter::File::Write
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    auto const max_offset
      { memory::make_offset (_file_by_id.at (segment_id)->_size)
      };

    if (max_offset < offset + size (range))
    {
      // \todo specific exception
      throw std::out_of_range
        { fmt::format ( "File::file_write: {} is not completely inside of {}"
                      , make_range (offset, size (range))
                      , make_range (memory::make_offset (0), max_offset)
                      )
        };
    }

    return memory::make_size
      ( util::Copy{}
        ( util::copy::FileReadLocation
            {filename (segment_id), make_off_t (offset)}
        , util::copy::FileWriteLocation
            {util::touch (path), make_off_t (begin (range))}
        , size_cast<std::size_t> (size (range))
        )
      );
  }
}
