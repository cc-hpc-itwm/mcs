// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/ranges.h>
#include <iterator>
#include <mcs/util/cast.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  constexpr auto Heap::Error::BadAlloc::requested
    (
    ) const noexcept -> memory::Size
  {
    return _requested;
  }

  constexpr auto Heap::Error::BadAlloc::used
    (
    ) const noexcept -> memory::Size
  {
    return _used;
  }

  constexpr auto Heap::Error::BadAlloc::max
    (
    ) const noexcept -> MaxSize
  {
    return _max;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Heap::Error::ChunkDescription::parameter
    (
    ) const noexcept -> Parameter::Chunk::Description const&
  {
    return _parameter;
  }

  constexpr auto Heap::Error::ChunkDescription::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }

  constexpr auto Heap::Error::ChunkDescription::memory_range
    (
    ) const noexcept -> core::memory::Range
  {
    return _memory_range;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    Heap::Chunk::Description<Access>::State::State
      ( Description<Access> const& description
      )
        : _bytes
          { memory::select
            ( typename Access::template Span<std::byte>
                { util::cast<typename Access::template Pointer<std::byte>>
                   (description._begin)
                , description._size
                }
            , description._range
            )
          }
  {}
  template<chunk::is_access Access>
    auto Heap::Chunk::Description<Access>::State::bytes
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _bytes;
  }

  template<chunk::is_access Access>
    auto Heap::chunk_description
      ( Parameter::Chunk::Description parameter
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  try
  {
    auto const buffer {_buffer_by_id.find (segment_id)};
    if (buffer == std::end (_buffer_by_id))
    {
      throw Error::ChunkDescription::UnknownSegmentID{};
    }
    auto const data
      { buffer->second
      . template data<typename Access::template ValueType<std::byte>>()
      };

    return Chunk::Description<Access>
      { util::cast<std::uintmax_t> (data.data())
      , data.size()
      , memory_range
      };
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::ChunkDescription {parameter, segment_id, memory_range}
      );
  }

  template<chunk::is_access Access>
    constexpr Heap::Chunk::Description<Access>::Description
      ( std::uintmax_t begin
      , std::size_t size
      , memory::Range range
      ) noexcept
        : _begin {begin}
        , _size {size}
        , _range {range}
  {}
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Tag>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Tag>::format
      ( mcs::core::storage::implementation::Heap::Tag const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::storage::implementation::Heap"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Tag>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Tag
  {
    prefix (state, "mcs::core::storage::implementation::Heap");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Tag>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Create>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap "
      , std::make_tuple (value.max_size)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Create
  {
    prefix (state, "Heap ");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Create>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Heap::Parameter::Create::max_size)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Size::Max>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Size::Max>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Size::Max const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Size::Max"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Size::Max>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Size::Max
  {
    prefix (state, "Heap::Size::Max");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Size::Max>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Size::Used>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Size::Used>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Size::Used const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Size::Used"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Size::Used>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Size::Used
  {
    prefix (state, "Heap::Size::Used");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Size::Used>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Segment::MLOCKed"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed
  {
    prefix (state, "Heap::Segment::MLOCKed");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::Create>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Segment::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Segment::Create "
      , std::make_tuple (value.mlocked)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Segment::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Segment::Create
  {
    prefix (state, "Heap::Segment::Create ");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Segment::Create>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::Heap::Parameter::Segment::Create::mlocked)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Segment::Remove>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Segment::Remove const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Segment::Remove"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Segment::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Segment::Remove
  {
    prefix (state, "Heap::Segment::Remove");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Segment::Remove>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Chunk::Description>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::Chunk::Description>::format
      ( mcs::core::storage::implementation::Heap::Parameter::Chunk::Description const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::Chunk::Description"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::Chunk::Description>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::Chunk::Description
  {
    prefix (state, "Heap::Chunk::Description");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::Chunk::Description>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::File::Read>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::File::Read>::format
      ( mcs::core::storage::implementation::Heap::Parameter::File::Read const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::File::Read"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::File::Read>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::File::Read
  {
    prefix (state, "Heap::File::Read");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::File::Read>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::File::Write>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Heap::Parameter::File::Write>::format
      ( mcs::core::storage::implementation::Heap::Parameter::File::Write const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Heap::File::Write"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Heap::Parameter::File::Write>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Heap::Parameter::File::Write
  {
    prefix (state, "Heap::File::Write");

    return std::make_from_tuple<mcs::core::storage::implementation::Heap::Parameter::File::Write>
      (parse<std::tuple<>> (state));
  }
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Heap::Chunk::Description<Access>>::output
      ( OArchive& oa
      , core::storage::implementation::Heap::Chunk::Description<Access> const& description
      ) -> OArchive&
  {
    save (oa, description._begin);
    save (oa, description._size);
    save (oa, description._range);

    return oa;
  }

  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::Heap::Chunk::Description<Access>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::Heap::Chunk::Description<Access>
  {
    using Description
      = core::storage::implementation::Heap::Chunk::Description<Access>
      ;

    auto _begin {load<decltype (Description::_begin)> (ia)};
    auto _size {load<decltype (Description::_size)> (ia)};
    auto _range {load<decltype (Description::_range)> (ia)};

    return Description {_begin, _size, _range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::Heap::Chunk::Description<Access>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::Heap::Chunk::Description<Access>>::format
        ( mcs::core::storage::implementation::Heap::Chunk::Description<Access> const& description
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "Heap::Chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( description._begin
        , description._size
        , description._range
        )
      );
  }
}
