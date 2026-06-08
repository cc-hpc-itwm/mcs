// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstring>
#include <mcs/serialization/detail/Tag.hpp>
#include <mcs/serialization/error/NotEnoughBytes.hpp>
#include <mcs/serialization/error/WrongTag.hpp>
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
      throw error::WrongTag{};
    }

    return std::get<TagType> (tag);
  }

  template<typename To>
    auto IArchive::extract (To to, std::size_t size) -> void
  {
    if (size > _buffer.size())
    {
      throw error::NotEnoughBytes {size, _buffer.size()};
    }

    std::memcpy (to, _buffer.data(), size);
    _buffer = _buffer.subspan (size);
  }
}
