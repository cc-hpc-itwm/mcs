// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

namespace mcs::serialization
{
  constexpr auto OArchive::sum_size_buffers() const noexcept -> std::size_t
  {
    return _sum_size_buffers;
  }

  template<typename TagType, typename... TagArgs>
    constexpr auto OArchive::tag
      ( TagArgs&&... tag_args
      ) -> OArchive&
  {
    return save
      ( *this
      , detail::Tag
        { std::in_place_type<TagType>
        , std::forward<TagArgs> (tag_args)...
        }
      );
  }

  template<typename T>
    auto OArchive::emplace_back (std::span<T> buffer) -> std::size_t
  {
    auto const size {_buffers.emplace_back (std::as_bytes (buffer)).size()};

    _sum_size_buffers += size;

    return size;
  }

  template<typename T>
    auto OArchive::append (std::span<T> buffer) -> OArchive&
  {
    _size_append += emplace_back (buffer);

    return *this;
  }

  template<typename T>
    auto OArchive::stream (std::span<T> buffer) -> OArchive&
  {
    std::ignore = emplace_back (buffer);

    return *this;
  }

  constexpr auto OArchive::buffers() const -> Buffers const&
  {
    return _buffers;
  }

  template<is_serializable... T>
    constexpr OArchive::OArchive (T const&... xs)
  {
    (save (*this, xs), ...);
  }

  template<is_serializable... T>
    constexpr OArchive::OArchive (T&... xs)
  {
    (save (*this, xs), ...);
  }

  template<is_serializable... T>
    constexpr OArchive::OArchive (T&&... xs)
  {
    (save (*this, std::forward<T> (xs)), ...);
  }

  template<typename Fun>
    requires (std::invocable<Fun, std::span<std::byte const>>)
    auto OArchive::for_each_data_buffer (Fun&& fun) const -> void
  {
    std::for_each
      ( std::next (std::begin (_buffers)), std::end (_buffers)
      , [&] (auto const& buffer)
        {
          std::invoke (std::forward<Fun> (fun), buffer);
        }
      );
  }
}
