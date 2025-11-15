// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <cstring>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/STD/optional.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/read/STD/optional.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <memory>
#include <tuple>
#include <utility>

namespace mcs::core::storage::implementation
{
  constexpr auto Files::Error::BadAlloc::requested
    (
    ) const noexcept -> memory::Size
  {
    return _requested;
  }

  constexpr auto Files::Error::BadAlloc::used
    (
    ) const noexcept -> memory::Size
  {
    return _used;
  }

  constexpr auto Files::Error::BadAlloc::max
    (
    ) const noexcept -> MaxSize
  {
    return _max;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Files::Error::ChunkDescription::parameter
    (
    ) const noexcept -> Parameter::Chunk::Description const&
  {
    return _parameter;
  }

  constexpr auto Files::Error::ChunkDescription::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }

  constexpr auto Files::Error::ChunkDescription::memory_range
    (
    ) const noexcept -> core::memory::Range
  {
    return _memory_range;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    Files::Chunk::Description<Access>::State::State
      ( Description<Access> const& description
      )
        : _open_file
            { new OpenFileImpl<Access>
              { description.path
              , Parameter::Segment::OnRemove::Keep{}
              , description.file_size
              }
            , typename OpenFileImpl<Access>::Deleter{}
            }
        , _bytes {memory::select (_open_file->data(), description.range)}
  {}
  template<chunk::is_access Access>
    auto Files::Chunk::Description<Access>::State::bytes
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _bytes;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto Files::chunk_description
      ( Parameter::Chunk::Description parameter
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  try
  {
    auto const path {filename (segment_id)};

    if (!std::filesystem::exists (path))
    {
      throw Error::ChunkDescription::UnknownSegmentID{};
    }

    return Chunk::Description<Access>
      { path
      , memory::make_size (std::filesystem::file_size (path))
      , memory_range
      };
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::ChunkDescription {parameter, segment_id, memory_range}
      );
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    Files::OpenFileImpl<Access>::Error::CouldNotOpen::CouldNotOpen
      ( std::filesystem::path path
      , char const* mode
      , int error_code
      )
        : mcs::Error
          { fmt::format
            ( "storage::implementation::Files::OpenFileImpl::CouldNotOpen: "
              "path '{}', mode '{}', error '{}'"
            , path
            , mode
            , strerror (error_code)
            )
          }
        , _path {path}
        , _mode {mode}
        , _error_code (error_code)
  {}
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Error::CouldNotOpen::path
      (
      ) const -> std::filesystem::path
  {
    return _path;
  }
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Error::CouldNotOpen::mode
      (
      ) const noexcept -> char const*
  {
    return _mode.data();
  }
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Error::CouldNotOpen::error_code
      (
      ) const noexcept -> int
  {
    return _error_code;
  }
  template<chunk::is_access Access>
    Files::OpenFileImpl<Access>::Error::CouldNotOpen::~CouldNotOpen() = default;

  template<chunk::is_access Access>
    Files::OpenFileImpl<Access>::Error::CouldNotCloseAfterTouch::CouldNotCloseAfterTouch
      ( std::filesystem::path path
      , int error_code
      )
        : mcs::Error
          { fmt::format
            ( "storage::implementation::Files::OpenFileImpl::CouldNotCloseAfterTouch: "
              "path '{}', error '{}'"
            , path
            , strerror (error_code)
            )
          }
        , _path {path}
        , _error_code (error_code)
  {}
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Error::CouldNotCloseAfterTouch::path
      (
      ) const -> std::filesystem::path
  {
    return _path;
  }
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Error::CouldNotCloseAfterTouch::error_code
      (
      ) const noexcept -> int
  {
    return _error_code;
  }
  template<chunk::is_access Access>
    Files::OpenFileImpl<Access>::Error::CouldNotCloseAfterTouch::~CouldNotCloseAfterTouch() = default;

  template<chunk::is_access Access>
    Files::OpenFileImpl<Access>::OpenFileImpl
      ( std::filesystem::path path
      , Parameter::Segment::Persistency persistency
      , memory::Size size
      )
        : _path {path}
        , _persistency {persistency}
        , _size {memory::size_cast<std::size_t> (size)}
        , _data
          { std::invoke
            ( [&]
              {
                touch();

                auto const file
                  { fopen ( chunk::select<Access>
                            ( chunk::make_value<chunk::access::Const> ("rb")
                            , chunk::make_value<chunk::access::Mutable> ("r+b")
                            )
                          )
                  };

                auto const close_file
                  { nonstd::make_scope_exit_that_dies_on_exception
                      ( "OpenFile::close_file"
                      , [&]
                        {
                          if (std::fclose (file) == EOF)
                          {
                            auto const error_code {errno};

                            throw mcs::Error
                              { fmt::format ( "Failure when closing '{}': {}"
                                            , _path
                                            , strerror (error_code)
                                            )
                              };
                          }
                        }
                      )
                  };

                return util::syscall::mmap_with_length_zero_allowed
                  ( nullptr
                  , _size
                  , chunk::select<Access>
                    ( chunk::make_value<chunk::access::Const>
                        (PROT_READ)
                    , chunk::make_value<chunk::access::Mutable>
                        (PROT_READ | PROT_WRITE)
                    )
                  , MAP_SHARED
                  , util::syscall::fileno (file)
                  , off_t {0}
                  );
              }
            )
          }
  {}
  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::remove_on_destruction() noexcept -> void
  {
    _persistency = Parameter::Segment::OnRemove::Remove{};
  }

  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::Deleter::operator()
      ( OpenFileImpl* open_file
      ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( fmt::format ( "Failure when removing '{}'"
                    , open_file->_path
                    )
      , [&]
        {
          util::syscall::munmap_with_length_zero_allowed
            ( open_file->_data
            , open_file->_size
            );

          std::visit
            ( util::overloaded
              ( [] (Parameter::Segment::OnRemove::Keep) noexcept
                {
                  // do nothing
                }
              , [&] (Parameter::Segment::OnRemove::Remove)
                {
                  std::filesystem::remove (open_file->_path);
                }
              )
            , open_file->_persistency
            );

          std::default_delete<OpenFileImpl>{} (open_file);
        }
      );
  }

  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::data
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return
      { static_cast<typename Access::template Pointer<std::byte>> (_data)
      , _size
      };
  }

  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::fopen
      ( char const* mode
      ) const -> std::FILE*
  {
    auto* const file {std::fopen (_path.c_str(), mode)};

    if (file == nullptr)
    {
      auto const error_code {errno};

      throw typename Error::CouldNotOpen {_path, mode, error_code};
    }

    return file;
  }

  template<chunk::is_access Access>
    auto Files::OpenFileImpl<Access>::touch() const -> void
  {
    if (std::fclose (fopen ("ab")) != 0)
    {
      auto const error_code {errno};

      throw typename Error::CouldNotCloseAfterTouch {_path, error_code};
    }

    std::filesystem::resize_file (_path, _size);
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "mcs::core::storage::implementation::Files"
  , mcs::core::storage::implementation::Files::Tag
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE
  ( "Prefix "
  , mcs::core::storage::implementation::Files::Prefix
  , value
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "Files "
  , mcs::core::storage::implementation::Files::Parameter::Create
  , prefix
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Size::Max"
  , mcs::core::storage::implementation::Files::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Size::Used"
  , mcs::core::storage::implementation::Files::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Segment::OnRemove::Keep"
  , mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Segment::OnRemove::Remove"
  , mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Files::Segment::Create "
  , mcs::core::storage::implementation::Files::Parameter::Segment::Create
  , persistency
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Segment::ForceRemoval"
  , mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Files::Segment::Remove "
  , mcs::core::storage::implementation::Files::Parameter::Segment::Remove
  , force_removal
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::Chunk::Description"
  , mcs::core::storage::implementation::Files::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::File::Read"
  , mcs::core::storage::implementation::Files::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Files::File::Write"
  , mcs::core::storage::implementation::Files::Parameter::File::Write
  );

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::storage::implementation::Files::Chunk::Description<Access>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, path);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, file_size);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, range);

    return oa;
  }

  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::Files::Chunk::Description<Access>
      )
  {
    using Description
      = core::storage::implementation::Files::Chunk::Description<Access>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, path, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, file_size, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, range, Description);

    return Description {path, file_size, range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::Files::Chunk::Description<Access>
      )
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( description
      , context
      , mcs::core::storage::implementation::Files::Chunk::Description<Access>
      )
  {
    return fmt::format_to
      ( context.out()
      , "Files::Chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( description.path
        , description.file_size
        , description.range
        )
      );
  }
}
