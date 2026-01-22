// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto Virtual::chunk_description
      ( Parameter::Chunk::Description parameter_chunk_description
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  {
    return _imported_c_api.template chunk_description<Access>
      ( parameter_chunk_description
      , segment_id
      , memory_range
      );
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Virtual::Error::Create::parameter
    (
    ) const noexcept -> Parameter::Create const&
  {
    return _parameter;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Virtual::Tag>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Virtual::Tag>::format
      ( mcs::core::storage::implementation::Virtual::Tag const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::storage::implementation::Virtual"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Virtual::Tag>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Virtual::Tag
  {
    prefix (state, "mcs::core::storage::implementation::Virtual");

    return std::make_from_tuple<mcs::core::storage::implementation::Virtual::Tag>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::Virtual::Parameter::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::Virtual::Parameter::Create>::format
      ( mcs::core::storage::implementation::Virtual::Parameter::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "Virtual "
      , std::make_tuple (value._shared_object, value._parameter_create)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::Virtual::Parameter::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::Virtual::Parameter::Create
  {
    prefix (state, "Virtual ");

    using Create = mcs::core::storage::implementation::Virtual::Parameter::Create;
    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::_shared_object)
               , decltype (Create::_parameter_create)
               >
             > (state)
      );
  }
}
