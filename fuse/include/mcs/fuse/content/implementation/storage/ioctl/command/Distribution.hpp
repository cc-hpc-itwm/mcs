// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <algorithm>
#include <array>
#include <mcs/fuse/content/implementation/storage/Part.hpp>
#include <mcs/util/cast.hpp>
#include <span>
#include <sys/ioctl.h>
#include <utility>
#include <vector>

namespace mcs::fuse::content::storage::ioctl::command
{
  struct Distribution
  {
    struct Buffer
    {
      [[nodiscard]] constexpr Buffer() noexcept = default;
      [[nodiscard]] Buffer
        ( Buffer const* in_buffer
        , std::vector<Part> const& parts
        )
          : _parts_index {in_buffer->_parts_index}
          , _number_of_parts_in_buffer {0}
      {
        if (_parts_index < parts.size())
        {
          auto const number_of_parts_to_copy
            { std::min
              ( parts.size() - _parts_index
              , MAX_NUMBER_OF_PARTS_PER_MESSAGE
              )
            };

          std::ranges::copy
            ( std::as_bytes
              ( std::span { parts.data() + _parts_index
                          , number_of_parts_to_copy
                          }
              )
            , _parts.data()
            );

          _parts_index += number_of_parts_to_copy;
          _number_of_parts_in_buffer += number_of_parts_to_copy;
        }
      }

      [[nodiscard]] constexpr operator bool() const noexcept
      {
        return _number_of_parts_in_buffer == MAX_NUMBER_OF_PARTS_PER_MESSAGE;
      }

      template<typename UsePart>
        requires (std::invocable<UsePart, Part const&>)
        auto for_each_part (UsePart&& use_part) const
      {
        std::ranges::for_each
          ( parts()
          , std::forward<UsePart> (std::forward<UsePart> (use_part))
          );
      }

    private:
      constexpr static auto MAX_NUMBER_OF_PARTS_PER_MESSAGE
        { std::size_t {512}
        };

      // in: starting index wanted
      // out: next index not in parts
      //
      std::size_t _parts_index {0};

      // out: parts are written by fuse
      //
      std::array<std::byte, MAX_NUMBER_OF_PARTS_PER_MESSAGE * sizeof (Part)>
        _parts{};

      // out: size used in parts
      //
      std::size_t _number_of_parts_in_buffer {MAX_NUMBER_OF_PARTS_PER_MESSAGE};

      [[nodiscard]] constexpr auto parts() const -> std::span<Part const>
      {
        return std::span
          { util::cast<Part const*> (_parts.data())
          , _number_of_parts_in_buffer
          };
      }
    };

    constexpr static auto CODE {_IOWR ('M', 0, Buffer)};
  };
}
