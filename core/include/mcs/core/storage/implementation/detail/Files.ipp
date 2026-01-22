// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <cstring>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/STD/optional.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/filesystem/path.hpp>
#include <mcs/util/read/STD/optional.hpp>
#include <mcs/util/read/STD/variant.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munmap.hpp>
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

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Tag>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Tag>::format
      ( mcs::core::storage::implementation::Files::Tag const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::storage::implementation::Files"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Tag>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Tag
  {
    prefix (state, "mcs::core::storage::implementation::Files");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Tag>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Prefix>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Prefix>::format
      ( mcs::core::storage::implementation::Files::Prefix const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}{}", "Prefix ", value.value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Prefix>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Prefix
  {
    prefix (state, "Prefix ");

    using Prefix = mcs::core::storage::implementation::Files::Prefix;

    return Prefix {parse<decltype (Prefix::value)> (state)};
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Create>::format
      ( mcs::core::storage::implementation::Files::Parameter::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files "
      , std::make_tuple (value.prefix, value.max_size)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Create
  {
    prefix (state, "Files ");

    using Create = mcs::core::storage::implementation::Files::Parameter::Create;
    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::prefix)
               , decltype (Create::max_size)
               >
             > (state)
      );
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Size::Max>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Size::Max>::format
      ( mcs::core::storage::implementation::Files::Parameter::Size::Max const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Size::Max"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Size::Max>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Size::Max
  {
    prefix (state, "Files::Size::Max");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Size::Max>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Size::Used>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Size::Used>::format
      ( mcs::core::storage::implementation::Files::Parameter::Size::Used const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Size::Used"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Size::Used>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Size::Used
  {
    prefix (state, "Files::Size::Used");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Size::Used>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>::format
      ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Segment::OnRemove::Keep"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep
  {
    prefix (state, "Files::Segment::OnRemove::Keep");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>::format
      ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Segment::OnRemove::Remove"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove
  {
    prefix (state, "Files::Segment::OnRemove::Remove");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Create>::format
      ( mcs::core::storage::implementation::Files::Parameter::Segment::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Segment::Create "
      , std::make_tuple (value.persistency)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Segment::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Segment::Create
  {
    prefix (state, "Files::Segment::Create ");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Segment::Create>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Files::Parameter::Segment::Create::persistency)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>::format
      ( mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Segment::ForceRemoval"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval
  {
    prefix (state, "Files::Segment::ForceRemoval");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>::format
      ( mcs::core::storage::implementation::Files::Parameter::Segment::Remove const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Segment::Remove "
      , std::make_tuple (value.force_removal)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Segment::Remove
  {
    prefix (state, "Files::Segment::Remove ");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Files::Parameter::Segment::Remove::force_removal)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>::format
      ( mcs::core::storage::implementation::Files::Parameter::Chunk::Description const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::Chunk::Description"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::Chunk::Description
  {
    prefix (state, "Files::Chunk::Description");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::File::Read>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::File::Read>::format
      ( mcs::core::storage::implementation::Files::Parameter::File::Read const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::File::Read"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::File::Read>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::File::Read
  {
    prefix (state, "Files::File::Read");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::File::Read>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::File::Write>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Files::Parameter::File::Write>::format
      ( mcs::core::storage::implementation::Files::Parameter::File::Write const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Files::File::Write"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Files::Parameter::File::Write>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Files::Parameter::File::Write
  {
    prefix (state, "Files::File::Write");

    return std::make_from_tuple<mcs::core::storage::implementation::Files::Parameter::File::Write>
      (parse<std::tuple<>> (state));
  }
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Files::Chunk::Description<Access>>::output
      ( OArchive& oa
      , core::storage::implementation::Files::Chunk::Description<Access> const& description
      ) -> OArchive&
  {
    save (oa, description.path);
    save (oa, description.file_size);
    save (oa, description.range);

    return oa;
  }

  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Files::Chunk::Description<Access>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::Files::Chunk::Description<Access>
  {
    using Description
      = core::storage::implementation::Files::Chunk::Description<Access>
      ;

    auto path {load<decltype (Description::path)> (ia)};
    auto file_size {load<decltype (Description::file_size)> (ia)};
    auto range {load<decltype (Description::range)> (ia)};

    return Description {path, file_size, range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::Files::Chunk::Description<Access>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::Files::Chunk::Description<Access>>::format
        ( mcs::core::storage::implementation::Files::Chunk::Description<Access> const& description
        , FormatContext& context
        ) const -> decltype (context.out())
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
