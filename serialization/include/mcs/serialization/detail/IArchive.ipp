// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstring>
#include <fmt/format.h>
#include <mcs/serialization/detail/Tag.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::serialization
{
  template<typename... BufferArgs>
    requires (std::is_constructible_v<IArchive::Buffer, BufferArgs...>)
    constexpr IArchive::IArchive (BufferArgs&&... buffer_args) noexcept
      : _buffer {std::forward<BufferArgs> (buffer_args)...}
  {}

  template<is_serializable TagType>
    auto IArchive::tag() -> TagType
  {
    auto const tag {load<detail::Tag> (*this)};

    if (!std::holds_alternative<TagType> (tag))
    {
      //! \todo specific exception for wrong tag type
      throw std::runtime_error {"IArchive::tag: Wrong tag type"};
    }

    return std::get<TagType> (tag);
  }

  template<typename To>
    auto IArchive::extract (To to, std::size_t size) -> void
  {
    if (size > _buffer.size())
    {
      //! \todo specific exception
      throw std::out_of_range
        { fmt::format
          ( "IArchive::extract: Out of range: {} > {}"
          , size
          , _buffer.size()
          )
        };
    }

    std::memcpy (to, _buffer.data(), size);
    _buffer = _buffer.subspan (size);
  }
}
